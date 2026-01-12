#include "../include/ParkingLogic.hpp"
#include "raylib.h"
#include <cstring>

// Vide car la gestion de l'occupation est faite par la simulation voiture
void UpdateParking(ParkingLot& p) {}

// Calcul la position centrale d'une place dans la grille du parking
Vector2 GetSpotPosition(const ParkingLot& p, int spotIndex) {
    float spotWidth = 24.0f;
    float spotHeight = 40.0f;
    float padding = 8.0f;
    int cols = (p.size.x - padding) / (spotWidth + padding);
    if (cols <= 0) cols = 1;

    int row = spotIndex / cols;
    int col = spotIndex % cols;

    float x = p.position.x + padding + col * (spotWidth + padding);
    float y = p.position.y + 10 + row * (spotHeight + padding);

    return { x + spotWidth / 2, y + spotHeight / 2 };
}



// Affichage graphique complet du parking avec places individuelles
void DrawParking(const ParkingLot& p) {
    // Surface du parking (bitume)
    DrawRectangleV(p.position, p.size, GetColor(0x2A2A2AFF));
    DrawRectangleLines(p.position.x, p.position.y, p.size.x, p.size.y, WHITE);

    // Panneau d'information avec nom et prix
    float xOffset = 0.0f;
    float yOffset = 0.0f;

    if (std::strcmp(p.name, "Central") == 0) {
        xOffset = -100.0f; 
    } else if (std::strcmp(p.name, "City") == 0) {
        xOffset = -100.0f; 
    } else if (std::strcmp(p.name, "Eco") == 0) {
        xOffset = 180.0f; 
    } else if (std::strcmp(p.name, "VIP") == 0) {
        xOffset = 150.0f; 
        yOffset = 80.0f;   
    }

    DrawRectangle(p.position.x + xOffset, p.position.y - 25 + yOffset, 100, 25, p.color);
    DrawText(p.name, p.position.x + 5 + xOffset, p.position.y - 22 + yOffset, 10, WHITE);
    DrawText(TextFormat("%.0fdh/h", p.price), p.position.x + 5 + xOffset, p.position.y - 10 + yOffset, 10, WHITE);

    float spotWidth = 24.0f;
    float spotHeight = 40.0f;
    float padding = 8.0f;
    int cols = (p.size.x - padding) / (spotWidth + padding);
    if (cols <= 0) cols = 1;

    // Dessin des places en grille
    for (int i = 0; i < p.capacity; i++) {
        int row = i / cols;
        int col = i % cols;

        float x = p.position.x + padding + col * (spotWidth + padding);
        float y = p.position.y + 10 + row * (spotHeight + padding);

        if (y + spotHeight > p.position.y + p.size.y) break;

        // Lignes blanches pour démarquer les places
        DrawRectangleLines(x, y, spotWidth, spotHeight, LIGHTGRAY);
    }

    for (int i = 0; i < p.capacity; ++i) {
    int row = i / cols;
    int col = i % cols;
    float x = p.position.x + padding + col * (spotWidth + padding);
    float y = p.position.y + 10 + row * (spotHeight + padding);
    if (y + spotHeight > p.position.y + p.size.y) break;

    if (p.spotsOccupied[i]) {
        // Place occupée : couleur voiture garée
        DrawRectangle(x + 2, y + 2, spotWidth - 4, spotHeight - 4, RED);
    } else {
        // Place libre : fond sombre (bitume)
        DrawRectangle(x + 2, y + 2, spotWidth - 4, spotHeight - 4, DARKGRAY);
    }
    DrawRectangleLines(x, y, spotWidth, spotHeight, WHITE);
    }
}