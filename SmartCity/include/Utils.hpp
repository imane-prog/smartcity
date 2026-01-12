#pragma once
#include "raylib.h"
#include "Components.hpp"   // Pour ParkingLot, Road
#include "Simulation.hpp"   

// Dessine une ligne pointillée entre deux points
void DrawDashedLine(Vector2 start, Vector2 end, float thickness, Color color);

// Dessine l’allée d’accès du parking à la route principale
void DrawDriveway(const struct ParkingLot& p, const struct Road& r);