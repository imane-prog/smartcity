#include "../include/Simulation.hpp"
#include "../include/ParkingLogic.hpp"
#include <cmath>
#include <limits>
#include <algorithm>

// Met à jour le cycle des feux tricolores (vert - jaune - rouge)
void TrafficLight::update(float dt) {
    timer -= dt;
    if (timer <= 0) {
        if (state == LIGHT_GREEN) {
            state = LIGHT_YELLOW;
            timer = 2.0f;
        } else if (state == LIGHT_YELLOW) {
            state = LIGHT_RED;
            timer = 5.0f;
        } else if (state == LIGHT_RED) {
            state = LIGHT_GREEN;
            timer = 5.0f;
        }
    }
}

// Vérifie si la voie est libre pour un changement de voie
bool IsLaneFree(const std::vector<Car>& cars, int roadIdx, int laneToCheck, float myDist, int myId) {
    for (const auto& other : cars) {
        if (other.id == myId) continue; // ne pas se comparer à soi-même
        if (other.roadIndex != roadIdx) continue;
        if (other.state != DRIVING) continue;
        if (other.currentLane == laneToCheck || other.targetLane == laneToCheck) {
            if (std::abs(other.distance - myDist) < SAFE_DISTANCE * 1.5f) return false;
        }
    }
    return true;
}

// Force la sortie de 2 voitures dans chaque parking plein pour libérer des places
void ForceExitFromFullParkings(std::vector<Car>& cars, std::vector<ParkingLot>& parkings) {
    for (int pidx = 0; pidx < (int)parkings.size(); pidx++) {
        ParkingLot& p = parkings[pidx];

        // Vérifier si toutes les places sont occupées
        bool isFull = true;
        for (bool occupied : p.spotsOccupied) {
            if (!occupied) {
                isFull = false;
                break;
            }
        }
    }
}

// Mise à jour principale de la simulation
void UpdateTraffic(std::vector<Car>& cars, std::vector<Road>& roads,
                   std::vector<ParkingLot>& parkings, float dt) {
    // Gestion des sorties forcées si parkings pleins
    ForceExitFromFullParkings(cars, parkings);

    // Mise à jour des feux sur chaque route
    for (auto& road : roads) road.light.update(dt);

    // Trier les voitures de l'avant vers l'arrière pour logique cohérente de déplacement
    std::sort(cars.begin(), cars.end(), [](const Car& a, const Car& b) {
        if (a.roadIndex == b.roadIndex)
            return a.distance > b.distance;
        return a.roadIndex < b.roadIndex;
    });

    for (auto& car : cars) {
        if (car.state == DRIVING) {
            if (car.waitTimer > 0) car.waitTimer -= dt; // UPDATE: Decrement timer in DRIVING

            Road& road = roads[car.roadIndex];
            float roadLength = road.getLength();

            Vector2 dir = road.getDir();
            Vector2 normal = { -dir.y, dir.x };
            Vector2 centerPos = Vector2Add(road.start, Vector2Scale(dir, car.distance));
            car.worldPos = Vector2Add(centerPos, Vector2Scale(normal, car.laneOffset));

            // Décision aléatoire d'aller se garer dans un parking disponible
            // UPDATE: Check timer to prevent immediate re-parking
            if (car.parkingIdx == -1 && car.waitTimer <= 0) {
                if (car.distance > 50 && GetRandomValue(0, 500) < 2) {
                    int bestIdx = -1;
                    float minDist = std::numeric_limits<float>::max();

                    // Restauration: On ne cherche que les parkings du même coté de la voie
                    int startIdx = 0; 
                    int endIdx = 0;
                    
                    if (car.roadIndex == 0) { // Road 1 (Top)
                        // Ln 0 (Top side) -> P0 (VIP)
                        // Ln 1 (Bottom side) -> P1 (Central Median)
                        if (car.currentLane == 0) { startIdx = 0; endIdx = 1; }
                        else { startIdx = 1; endIdx = 2; }
                    } else { // Road 2 (Bottom)
                        // Ln 0 (Bottom side) -> P2, P3
                        // Ln 1 (Top side) -> Rien (ou P1 aussi ? Non restons simple)
                        if (car.currentLane == 0) { startIdx = 2; endIdx = 4; }
                        else { startIdx = 0; endIdx = 0; }
                    }
                    
                    for (int i = startIdx; i < endIdx && i < (int)parkings.size(); i++) {
                        // ... (same loop) ...
                        if (parkings[i].firstFreeSpot() != -1) { 
                            float dist = Vector2Distance(car.worldPos, parkings[i].position);
                            if (dist < minDist) {
                                minDist = dist;
                                bestIdx = i;
                            }
                        }
                    }
                    if (bestIdx != -1) {
                         // Restriction VIP (Index 0) : Max 2 voitures
                         if (bestIdx == 0) {
                             int vipCount = 0;
                             for (const auto& other : cars) {
                                 if (other.parkingIdx == 0) vipCount++;
                             }
                             if (vipCount >= 1) {
                                 bestIdx = -1; // Trop cher/plein pour ce pauvre conducteur
                             }
                         }
                         if (bestIdx != -1) car.parkingIdx = bestIdx;
                    }
                }
            }


            // Approche parking selon la voie autorisée sur chaque parking
            if (car.parkingIdx != -1) {
                ParkingLot& p = parkings[car.parkingIdx];
                float entranceX = p.position.x + p.size.x / 2;

                bool correctLane = false;
                if (car.parkingIdx == 0)      correctLane = (car.currentLane == 0); // VIP -> Ln 0
                else if (car.parkingIdx == 1) correctLane = (car.currentLane == 1); // Central -> Ln 1
                else                          correctLane = (car.currentLane == 0); // Eco/City -> Ln 0 (sur R2)

                if (correctLane) {
                    if (std::abs(car.worldPos.x - entranceX) < 10.0f) {
                        int spot = p.firstFreeSpot();
                        if (spot != -1) {
                            car.state = TO_PARKING;
                            car.spotIdx = spot;
                            car.targetPos = GetSpotPosition(p, spot);
                            p.occupySpot(spot);
                        } else {
                            car.parkingIdx = -1;
                        }
                    }
                } else {
                    car.parkingIdx = -1; // Mauvaise voie, on annule
                }
            }

            // Détection obstacle devant pour la voiture sur la route et freinage/ralentissement adapté
            float distToObstacle = std::numeric_limits<float>::max();
            for (const auto& other : cars) {
                if (car.id == other.id) continue;

                // On ignore les voitures garées
                if (other.state == PARKED) continue;
                
                
                
                if (car.roadIndex == other.roadIndex && car.currentLane == other.currentLane) {
                    // Si other est LEAVING, sa distance n'est pas encore sur la route... 
                    // Sauf si on considère sa position projetée ?
                    // Le plus critique est TO_PARKING qui disparait subitement.
                    
                    if (other.state == DRIVING || other.state == TO_PARKING) {
                         if (other.distance > car.distance) {
                            float d = other.distance - car.distance;
                            if (d < distToObstacle)
                                distToObstacle = d;
                        }
                    }
                    else if (other.state == LEAVING_PARKING) {
                         // Pour une voiture qui sort, sa position sur la route est approximée par sa cible (le point de sortie)
                         // On projette targetPos sur la route
                         Road& r = roads[other.roadIndex];
                         Vector2 dir = r.getDir();
                         Vector2 v = Vector2Subtract(other.targetPos, r.start);
                         float otherDist = v.x * dir.x + v.y * dir.y;

                         if (otherDist > car.distance) {
                             float d = otherDist - car.distance;
                             if (d < distToObstacle)
                                 distToObstacle = d;
                         }
                    }
                }
            }

            // Impact du feu sur la vitesse
            if (road.light.state != LIGHT_GREEN) {
                float distToLight = (roadLength - 200.0f) - car.distance;
                if (distToLight > 0 && distToLight < distToObstacle)
                    distToObstacle = distToLight;
            }

            // Calcul et application de la vitesse cible
            float targetSpeed = MAX_SPEED;
            // La voiture fait 40px de long. Distance centre à centre min = 40.
            // On freine d'urgence si on est trop près (< 45px pour laisser 5px de marge)
            if (distToObstacle < 45.0f) {
                car.speed = 0.0f; // freinage d'urgence
            } else if (distToObstacle < SAFE_DISTANCE) {
                targetSpeed = 0.0f;
                car.speed = Lerp(car.speed, targetSpeed, 10.0f * dt);
            } else if (distToObstacle < SAFE_DISTANCE * 2.5f) {
                targetSpeed = MAX_SPEED * 0.3f;
                car.speed = Lerp(car.speed, targetSpeed, 5.0f * dt);
            } else {
                car.speed = Lerp(car.speed, MAX_SPEED, 5.0f * dt);
            }
            if (car.parkingIdx != -1)
                car.speed = std::min(car.speed, 80.0f);

            car.distance += car.speed * dt;

            if (car.distance > roadLength + 50) {
                car.distance = -CAR_LENGTH;
                car.speed = MAX_SPEED;
                car.parkingIdx = -1;
                car.roadIndex = 1 - car.roadIndex; // Switch to the other road (Loop)
            }
        }
        else if (car.state == TO_PARKING) {
            // Détection obstacle pour freinage progressif
            float distToObstacle = std::numeric_limits<float>::max();
            
            for (const auto& other : cars) {
                if (other.id == car.id) continue;
                
                // 1. Obstacle DANS le parking (qui entre ou sort)
                if ((other.state == TO_PARKING || other.state == LEAVING_PARKING) && other.parkingIdx == car.parkingIdx) {
                    float dist = Vector2Distance(car.worldPos, other.worldPos);
                    // Vision Cone (~45 deg)
                    Vector2 myDir = Vector2Subtract(car.targetPos, car.worldPos);
                    Vector2 toOther = Vector2Subtract(other.worldPos, car.worldPos);
                    
                    if (Vector2DotProduct(Vector2Normalize(myDir), Vector2Normalize(toOther)) > 0.7f) {
                         if (dist < distToObstacle) distToObstacle = dist;
                    }
                }
                
                // 2. Obstacle SUR LA ROUTE (embouteillage entrée)
                if (other.state == DRIVING && other.roadIndex == car.roadIndex && other.currentLane == car.currentLane) {
                     float d = other.distance - car.distance;
                     if (d > 0 && d < 60.0f) {
                         // On convertit cette distance road-based en distance physique approx
                         if (d < distToObstacle) distToObstacle = d;
                     }
                }
            }

            // Calcul vitesse progressive
            float targetSpeed = 80.0f; // Vitesse max parking
            if (distToObstacle < 45.0f) {
                targetSpeed = 0.0f; // Arrêt complet
            } else if (distToObstacle < 100.0f) {
                // Freinage linéaire entre 100px et 45px
                float factor = (distToObstacle - 45.0f) / (100.0f - 45.0f);
                targetSpeed = 80.0f * factor;
            }

            // Lissage de la vitesse
            car.speed = Lerp(car.speed, targetSpeed, 10.0f * dt);
            
            // Application du mouvement (si on roule)
            if (car.speed > 0.1f) {
                // Mouvement "Manhattan" : On s'aligne en X d'abord, puis on entre en Y.
                float dx = car.targetPos.x - car.worldPos.x;
                float dy = car.targetPos.y - car.worldPos.y;
                float step = car.speed * dt;

                // Phase 1 : Alignement horizontal (Allée)
                if (std::abs(dx) > 2.0f) {
                    car.worldPos.x += (dx > 0 ? step : -step);
                    car.rotation = (dx > 0) ? 0.0f : 180.0f;
                } 
                // Phase 2 : Entrée dans la place (Vertical)
                else {
                    car.worldPos.x = car.targetPos.x; 
                    if (std::abs(dy) > 2.0f) {
                        car.worldPos.y += (dy > 0 ? step : -step);
                        car.rotation = (dy > 0) ? 90.0f : 270.0f;
                    } else {
                        // Arrivé
                        car.state = PARKED;
                        car.waitTimer = GetRandomValue(15, 25);
                        car.worldPos = car.targetPos;
                        car.speed = 0;
                    }
                }
            }
        }


       else if (car.state == PARKED) {
            car.waitTimer -= dt;
            if (car.waitTimer <= 0) {
                // 1. Définir la cible de sortie (exitPos)
                car.targetPos = parkings[car.parkingIdx].exitPos; 

                // --- SECURITE : Vérifier si la voie est libre avant de sortir ---
                // On projette la position de sortie sur la route pour estimer la distance
                Road& road = roads[car.roadIndex];
                Vector2 roadDir = road.getDir();
                Vector2 exitVec = Vector2Subtract(car.targetPos, road.start);
                float projectedDist = (exitVec.x * roadDir.x + exitVec.y * roadDir.y);
                
                // On vérifie la voie de droite 
                // P0 (Top/Left) -> Lane 0
                // P1 (Median/Right) -> Lane 1
                // P2, P3 (Bottom/Left of R2) -> Lane 0
                int exitLane = (car.parkingIdx == 1) ? 1 : 0;

                // Check 1: Est-ce que quelqu'un d'autre est DÉJÀ en train de sortir de ce parking ?
                bool someoneExiting = false;
                for(const auto& other : cars) {
                     if (other.id != car.id && other.parkingIdx == car.parkingIdx && other.state == LEAVING_PARKING) {
                         someoneExiting = true;
                         break;
                     }
                }

                // Check 2: Est-ce que la route est "vide" (grand espace libre) ?
                bool isRoadClear = true;
                if (!someoneExiting) {
                    for (const auto& other : cars) {
                        if (other.id == car.id) continue;
                        
                        // On vérifie DRIVING et TO_PARKING sur TOUTE LA ROUTE (toutes les voies)
                        // Si la route est "pleine" (même sur l'autre voie), on attend.
                        if (other.roadIndex == car.roadIndex && 
                           (other.state == DRIVING || other.state == TO_PARKING)) {
                            
                            float diff = projectedDist - other.distance;
                            
                            // Voitures arrivant de derrière (Upstream)
                            // On utilise délibérément une marge LARGE basée sur SAFE_DISTANCE
                            if (diff > 0 && diff < SAFE_DISTANCE * 6.0f) { 
                                isRoadClear = false; 
                                break;
                            }
                            
                            // Voitures juste devant (Downstream)
                            if (diff < 0 && diff > -SAFE_DISTANCE * 3.0f) { 
                                isRoadClear = false; 
                                break;
                            }
                        }
                    }
                }

                if (!someoneExiting && isRoadClear) {
                    car.state = LEAVING_PARKING;
                } else {
                    // Si pas libre, on attend encore un peu
                    car.waitTimer = 1.0f; 
                }
            }
        }
  else if (car.state == LEAVING_PARKING) {
    float roadY = (car.roadIndex == 0) ? 250.0f : 600.0f;
    float dy = roadY - car.worldPos.y;
    float moveSpeed = 80.0f * dt;

    // PHASE 1: sortis vertical
    if (std::abs(dy) > 2.0f) {
        
        
        car.rotation = (dy < 0) ? 270.0f : 90.0f; 
        car.worldPos.y += (dy > 0 ? moveSpeed : -moveSpeed);
        continue; // CORRECTIF : continue au lieu de return pour ne pas bloquer les autres voitures
    }

    // PHASE 2: rotation pour se mettre dans la route
    
    if (std::abs(car.rotation) > 2.0f && std::abs(car.rotation) < 358.0f) {
        float rotSpeed = 400.0f * dt;
        
    //horizontalement dans la route
        if (car.rotation > 180.0f) car.rotation += rotSpeed; 
        else car.rotation -= rotSpeed; 
        // N-hadiw l-angle bach ma-i-foutch 360
        if (car.rotation >= 360.0f) car.rotation = 0.0f;
        if (car.rotation < 0.0f) car.rotation = 0.0f;
        
        continue; // CORRECTIF : continue au lieu de return
    }

    // PHASE 3: REPRENDRE LA ROUTE (Driving)
    car.worldPos.y = roadY;
    car.rotation = 0.0f; // Fixation finale
    car.state = DRIVING;

    Road& road = roads[car.roadIndex];
    Vector2 roadDir = road.getDir();
    Vector2 carVec = Vector2Subtract(car.worldPos, road.start);
    car.distance = (carVec.x * roadDir.x + carVec.y * roadDir.y);

    if (car.parkingIdx != -1) {
        parkings[car.parkingIdx].freeSpot(car.spotIdx);
    }
    
    car.parkingIdx = -1;
    car.speed = 50.0f;
    // UPDATE: Ajouter un cooldown pour ne pas rentrer directement dans le parking
    car.waitTimer = 10.0f; 
}
}
    }

