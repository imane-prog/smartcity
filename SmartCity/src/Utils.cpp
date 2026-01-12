#include "../include/Utils.hpp"
#include "raymath.h"
void DrawDashedLine(Vector2 start, Vector2 end, float thickness, Color color) {
    float length = Vector2Distance(start, end);
    Vector2 dir = Vector2Normalize(Vector2Subtract(end, start));
    for (float i = 0; i < length; i += 40.0f) {
        float len = (i + 20 > length) ? (length - i) : 20;
        DrawLineEx(Vector2Add(start, Vector2Scale(dir, i)),
                   Vector2Add(start, Vector2Scale(dir, i + len)), thickness, color);
    }
}

void DrawDriveway(const ParkingLot& p, const Road& r) {
    float centerX = p.position.x + p.size.x / 2;
    Vector2 roadPoint = { centerX, r.start.y };
    float parkingY = (p.position.y < r.start.y) ? (p.position.y + p.size.y) : p.position.y;
    Vector2 parkingPoint = { centerX, parkingY };

    DrawLineEx(parkingPoint, roadPoint, p.size.x, GetColor(0x333333FF));
    DrawLineEx({ centerX - p.size.x/2, parkingPoint.y }, { centerX - p.size.x/2, roadPoint.y }, 2.0f, GRAY);
    DrawLineEx({ centerX + p.size.x/2, parkingPoint.y }, { centerX + p.size.x/2, roadPoint.y }, 2.0f, GRAY);
}