#pragma once
#include "Components.hpp"
#include <vector>

// Vérifie si la voie est libre (pour changement de voie)
bool IsLaneFree(const std::vector<Car>& cars, int roadIdx, int laneToCheck, float myDist, int myId);

// Mise à jour complète du trafic automobile, y compris parkings
void UpdateTraffic(std::vector<Car>& cars, std::vector<Road>& roads, std::vector<ParkingLot>& parkings, float dt);