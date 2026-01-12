#include "../include/CarLogic.hpp"
#include "raylib.h"
#include "raymath.h"
#include <cmath>
void DrawCar(const Car& car, const Road& road) {
    Vector2 pos;
    float angle;
//logic de position de la voiture driving ou stationnement 
    if (car.state == DRIVING) {
        Vector2 dir = road.getDir();
        Vector2 normal = { -dir.y, dir.x };
        Vector2 centerPos = Vector2Add(road.start, Vector2Scale(dir, car.distance));
        pos = Vector2Add(centerPos, Vector2Scale(normal, car.laneOffset));
        angle = atan2(dir.y, dir.x) * RAD2DEG;
    } else {
        pos = car.worldPos;
        
        angle = car.rotation;
    }

    // --- PARAMETRES ---
    float length = 38.0f;
    float width = 20.0f;
    float rad = angle * DEG2RAD;
    Color bodyColor = car.color;

    // 1. LES ROUE
    float wheelW = 8.0f; float wheelH = 4.0f;
    Vector2 wheelOffsets[4] = { {12,-9}, {12,9}, {-12,-9}, {-12,9} };
    for(int i=0; i<4; i++) {
        Rectangle wRec = { pos.x, pos.y, wheelW, wheelH };
        DrawRectanglePro(wRec, { -wheelOffsets[i].x + wheelW/2, -wheelOffsets[i].y + wheelH/2 }, angle, BLACK);
    }

    // 2. CORPS 
    Rectangle body = { pos.x, pos.y, length, width };
    DrawRectanglePro(body, { length/2, width/2 }, angle, bodyColor);
    
    // CONTOUR 
    
    DrawPolyLinesEx(pos, 4, length/1.8f, angle + 45, 2, Fade(BLACK, 0.4f));

    // 3. Windows/Glass
    Rectangle glass = { pos.x, pos.y, length * 0.45f, width * 0.75f };
    DrawRectanglePro(glass, { (length * 0.45f)/2 - 3, (width * 0.75f)/2 }, angle, GetColor(0x222222FF));

    // 4. PHARES AVANT (Yellow)
    Vector2 front = { pos.x + cosf(rad) * (length/2), pos.y + sinf(rad) * (length/2) };
    Vector2 side = { -sinf(rad) * (width/3), cosf(rad) * (width/3) };
    DrawCircleV(Vector2Add(front, side), 3, YELLOW);
    DrawCircleV(Vector2Subtract(front, side), 3, YELLOW);

    // 5. FEUX ARRIÈRE (Red)
    Vector2 back = { pos.x - cosf(rad) * (length/2), pos.y - sinf(rad) * (length/2) };
    DrawCircleV(Vector2Add(back, side), 2, RED);
    DrawCircleV(Vector2Subtract(back, side), 2, RED);
}
