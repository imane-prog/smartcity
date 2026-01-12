#include <iostream>
#include <vector>
#include <limits>
#include "../include/Simulation.hpp"

// 1. Les fonctions utilitaires doivent être en dehors de tout bloc
Road CreateDummyRoad() {
    Road r;
    r.start = {0, 0};
    r.end = {1000, 0};
    r.light.state = LIGHT_GREEN;
    r.light.timer = 5.0f;
    return r;
}

// 2. Test du feu rouge (Version corrigée avec détection)
void TestFeuRouge() {
    Road r = CreateDummyRoad();
    r.end = {500, 0}; 
    r.light.state = LIGHT_RED; // On force le feu rouge
    
    Car c;
    c.id = 99;
    c.speed = 100.0f;
    c.distance = 460.0f; // 40px avant le feu (dans la zone de détection)
    c.roadIndex = 0;
    c.state = DRIVING;

    std::vector<Car> cars = {c};
    std::vector<Road> roads = {r};
    std::vector<ParkingLot> parkings;

    // Simulation de 5 pas pour laisser le Lerp agir
    for(int i = 0; i < 5; i++) {
        UpdateTraffic(cars, roads, parkings, 0.1f);
    }

    if (cars[0].speed < 100.0f) {
        std::cout << "[OK] Test Feu Rouge Reussi. Vitesse : " << cars[0].speed << std::endl;
    } else {
        std::cout << "[FAIL] La voiture n'a pas freine. Vitesse : " << cars[0].speed << std::endl;
    }
}
// --- DÉFINITION : TEST COLLISION ---
void TestCollision() {
    Road r; r.start = {0, 100}; r.end = {1000, 100};
    std::vector<Road> roads = {r};

    Car obstacle; obstacle.worldPos = {200, 100}; obstacle.speed = 0; obstacle.roadIndex = 0;
    Car suiveuse; suiveuse.worldPos = {180, 100}; suiveuse.speed = 50.0f; suiveuse.id = 1; suiveuse.roadIndex = 0;

    std::vector<Car> cars = {obstacle, suiveuse};
    std::vector<ParkingLot> parkings;

    UpdateTraffic(cars, roads, parkings, 0.1f);

    if (cars[1].speed < 50.0f)
        std::cout << "[OK] Test Collision Reussi." << std::endl;
}

// 3. Test de l'entrée au parking
void TestEntreeParking() {
    std::vector<Road> roads = { CreateDummyRoad() };
    std::vector<Car> cars;
    std::vector<ParkingLot> parkings;

    // Utilisation du constructeur à 7 arguments
    ParkingLot p({100, 100}, {50, 50}, 1, 2.0f, "TestPark", GRAY, {110, 110});
    p.spotsOccupied = {false}; 
    parkings.push_back(p);

    Car c;
    c.id = 1;
    c.worldPos = {105, 100}; // Position proche de l'entrée
    c.distance = 100.0f;
    c.state = DRIVING;
    c.parkingIdx = 0; // Veut aller au parking 0
    c.roadIndex = 0;
    c.currentLane = 0; // Lane 0 required for P0
    cars.push_back(c);

    for(int i = 0; i < 5; i++) {
        UpdateTraffic(cars, roads, parkings, 0.1f);
        if(cars[0].state == TO_PARKING) break;
    }

    if (cars[0].state == TO_PARKING) {
        std::cout << "[OK] Test Parking Reussi." << std::endl;
    } else {
        std::cout << "[FAIL] La voiture ne se gare pas. Etat : " << (int)cars[0].state << std::endl;
    }
}

// 5. Test de la sortie de parking (Regression Test)
void TestExitParking() {
    std::cout << "--- TestExitParking ---" << std::endl;
    std::vector<Road> roads = { CreateDummyRoad() };
    std::vector<Car> cars;
    std::vector<ParkingLot> parkings;

    ParkingLot p({100, 100}, {50, 50}, 1, 2.0f, "TestPark", GRAY, {110, 110});
    p.spotsOccupied = {true}; // Place occupée
    parkings.push_back(p);

    Car c;
    c.id = 1;
    c.worldPos = {100, 100}; // Sur la place
    c.targetPos = {100, 100};
    c.state = PARKED;
    c.parkingIdx = 0;
    c.spotIdx = 0;
    c.waitTimer = 0.05f; // Timer très court
    cars.push_back(c);

    // Update traffic
    for(int i = 0; i < 5; i++) {
        UpdateTraffic(cars, roads, parkings, 0.1f);
    }

    // Après expiration du timer, état attendu : LEAVING_PARKING (et NON pas DRIVING direct)
    if (cars[0].state == LEAVING_PARKING) {
        std::cout << "[OK] La voiture sort doucement (State = LEAVING_PARKING)." << std::endl;
    } else if (cars[0].state == DRIVING) {
        std::cout << "[FAIL] La voiture a saute sur la route (State = DRIVING)." << std::endl;
    } else {
        std::cout << "[INFO] Etat actuel : " << (int)cars[0].state << std::endl;
    }
}

// 6. Test de collision en sortie de parking
void TestExitParkingCollision() {
    std::cout << "--- TestExitParkingCollision ---" << std::endl;
    std::vector<Road> roads = { CreateDummyRoad() };
    std::vector<Car> cars;
    std::vector<ParkingLot> parkings;

    ParkingLot p({100, 100}, {50, 50}, 1, 2.0f, "TestPark", GRAY, {110, 110});
    p.spotsOccupied = {true};
    parkings.push_back(p); // Index 0
    parkings.push_back(p); // Index 1 (Added to prevent crash when using parkingIdx=1)

    // Voiture 1 : Garée, prête à sortir
    Car c1;
    c1.id = 1;
    c1.worldPos = {100, 100};
    c1.targetPos = {100, 100};
    c1.state = PARKED;
    c1.parkingIdx = 1;
    c1.spotIdx = 0;
    c1.waitTimer = 0.0f; // Sort immédiate
    c1.roadIndex = 0;
    c1.currentLane = 0; 
    cars.push_back(c1);

    // Voiture 2 : Sur la route, bloque la sortie
    Car c2;
    c2.id = 2;
    c2.state = DRIVING;
    c2.roadIndex = 0;
    c2.currentLane = 1; 
    c2.distance = 110.0f; 
    c2.speed = 0.0f; // Elle est arrêtée juste devant
    c2.laneOffset = 0; // Dans la lane 0
    cars.push_back(c2);

    // Mise à jour
    UpdateTraffic(cars, roads, parkings, 0.1f);

    // Retrouver la voiture 1 (celle qui était garée)
    Car* pC1 = nullptr;
    for (auto& c : cars) {
        if (c.id == 1) {
            pC1 = &c;
            break;
        }
    }

    if (pC1) {
        if (pC1->state == LEAVING_PARKING) {
            std::cout << "[FAIL] La voiture sort alors que la route est bloquee!" << std::endl;
        } else if (pC1->state == PARKED) {
             std::cout << "[OK] La voiture attend que la route se libere." << std::endl;
        } else {
            std::cout << "[INFO] Etat inattendu: " << (int)pC1->state << std::endl;
        }
    } else {
        std::cout << "[ERROR] Voiture 1 introuvable apres update." << std::endl;
    }
}

// 4. Point d'entrée principal
void TestEnterParkingCollision() {
    std::cout << "--- TestEnterParkingCollision ---" << std::endl;
    std::vector<Road> roads = { CreateDummyRoad() };
    std::vector<Car> cars;
    std::vector<ParkingLot> parkings;

    ParkingLot p({100, 100}, {50, 50}, 1, 2.0f, "TestPark", GRAY, {110, 110});
    p.spotsOccupied = {false};
    parkings.push_back(p);

    // Voiture 1 : Rentre au parking
    Car c1;
    c1.id = 1;
    c1.roadIndex = 0;
    c1.currentLane = 0; 
    c1.distance = 125.0f; // EXACTLY at entrance (100 + 50/2)
    c1.state = DRIVING; // Will switch to TO_PARKING
    c1.parkingIdx = 0; 
    c1.worldPos = {125, 80}; // Align to Lane 0 (y=80)
    c1.laneOffset = -10; // offset for Lane 0? DummyRoad width 40. Normal(0,1). Ln0 is -10. Ln1 is +10.
    c1.targetPos = {0,0}; // Init
    
    // Voiture 2 : Suit de près
    Car c2;
    c2.id = 2;
    c2.roadIndex = 0;
    c2.currentLane = 0; // Following in same lane (Lane 0)
    c2.distance = 105.0f; // 20m derrière.
    
    c2.speed = 100.0f;
    c2.state = DRIVING;
    c2.parkingIdx = -1;

    cars.push_back(c1);
    cars.push_back(c2);

    // Update 1 frame to let c1 switch to TO_PARKING
    UpdateTraffic(cars, roads, parkings, 0.1f);
    
    // Now c1 should be TO_PARKING
    // Check if c2 detects c1
    // We update again
    UpdateTraffic(cars, roads, parkings, 0.1f);

    // Retrouver c2
    Car* pC2 = nullptr;
    for (auto& c : cars) { if (c.id == 2) pC2 = &c; }

    if (pC2) {
        // Si c2 a freiné, speed < 100
        if (pC2->speed < 95.0f) {
             std::cout << "[OK] La voiture suiveuse a freine." << std::endl;
        } else {
             std::cout << "[FAIL] La voiture suiveuse n'a pas freine (Speed=" << pC2->speed << ")." << std::endl;
        }
    }
}

void TestDrivingVsLeavingCollision() {
    std::cout << "--- TestDrivingVsLeavingCollision ---" << std::endl;
    std::vector<Road> roads = { CreateDummyRoad() };
    std::vector<Car> cars;
    std::vector<ParkingLot> parkings;

    ParkingLot p({100, 100}, {50, 50}, 1, 2.0f, "TestPark", GRAY, {110, 110});
    p.spotsOccupied = {false};
    parkings.push_back(p);


    // Voiture 1 (Exiting)
    Car c1;
    c1.id = 1;
    c1.state = LEAVING_PARKING; 
    c1.parkingIdx = 1; // Use 1 now (Median) to be consistent with Lane 1? Yes, Lane 1 matches P1.
    c1.roadIndex = 0;
    c1.currentLane = 1; 
    c1.worldPos = {105, 105}; 
    c1.targetPos = {110, 110}; 
    c1.distance = 0; 
    c1.laneOffset = 0;

    // Voiture 2 (Driving)
    Car c2;
    c2.id = 2;
    c2.state = DRIVING;
    c2.roadIndex = 0;
    c2.currentLane = 1;
    c2.distance = 80.0f; 
    c2.speed = 100.0f; 
    c2.parkingIdx = -1;

    cars.push_back(c1);
    cars.push_back(c2);

    UpdateTraffic(cars, roads, parkings, 0.1f);

    Car* pC2 = nullptr;
    for (auto& c : cars) { if (c.id == 2) pC2 = &c; }

    if (pC2) {
        if (pC2->speed < 95.0f) {
             std::cout << "[OK] La voiture suiveuse a freine pour celle qui sort." << std::endl;
        } else {
             std::cout << "[FAIL] La voiture suiveuse ignore celle qui sort (Speed=" << pC2->speed << ")." << std::endl;
        }
    }
}

int main() {
    std::cout << "===== LANCEMENT DES TESTS =====" << std::endl;
    TestFeuRouge();
    TestEntreeParking();
    TestCollision(); 
    TestExitParking();
    TestExitParkingCollision();
    TestEnterParkingCollision();
    TestDrivingVsLeavingCollision();
    std::cout << "===== TOUS LES TESTS SONT FINIS =====" << std::endl;
    return 0;
}