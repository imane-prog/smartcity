#pragma once
#include "Components.hpp"

// Met à jour le parking (étoffé ici, vide car la gestion est faite par voitures)
void UpdateParking(ParkingLot& p);

// Dessine le parking avec ses places et panneaux infos
void DrawParking(const ParkingLot& p);

// Calcule la position finale (x,y) d'une place donnée dans un parking
Vector2 GetSpotPosition(const ParkingLot& p, int spotIndex);