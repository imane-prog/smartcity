#pragma once
#include "raylib.h"
#include "raymath.h"
#include <vector>

const float CAR_LENGTH = 40.0f;
const float CAR_WIDTH = 20.0f;
const float SAFE_DISTANCE = 160.0f;
const float MAX_SPEED = 200.0f;

enum LightState { LIGHT_GREEN, LIGHT_YELLOW, LIGHT_RED };
enum CarState { DRIVING, TO_PARKING, PARKED, LEAVING_PARKING };

struct TrafficLight {
    Vector2 position;
    LightState state;
    float timer;
    void update(float dt);
};

struct Road {
    Vector2 start;
    Vector2 end;
    int lanes;
    float width;
    TrafficLight light;

    float getLength() const { return Vector2Distance(start, end); }
    Vector2 getDir() const { return Vector2Normalize(Vector2Subtract(end, start)); }
};

struct Car {
    int id;
    int roadIndex;
    float rotation;
    int currentLane;
    float distance;
    float speed;
    Color color;

    float laneOffset;
    int targetLane;
    float laneChangeTimer;

    CarState state;
    Vector2 worldPos;
    Vector2 targetPos;
    float waitTimer;
    int parkingIdx;
    int spotIdx;

    // Constructeur pour initialiser proprement
    Car() : id(0), roadIndex(0), currentLane(0), distance(0), speed(0), color(RED),
            laneOffset(0), targetLane(0), laneChangeTimer(0),
            state(DRIVING), worldPos({0,0}), targetPos({0,0}),
            waitTimer(0), parkingIdx(-1), spotIdx(-1) {}
};

struct ParkingLot {
    Vector2 position;
    Vector2 size;
    int capacity;
    std::vector<bool> spotsOccupied;
    float price;
    const char* name;
    Color color;
    Vector2 exitPos;
    

    // Constructeur bien défini
    
    ParkingLot(Vector2 pos, Vector2 sz, int cap, float pr, const char* nm, Color col, Vector2 exit)
    : position(pos), size(sz), capacity(cap), price(pr), name(nm), color(col), exitPos(exit), spotsOccupied(cap, false) 
    {}

    // Méthodes membre
    int firstFreeSpot() const {
        for (int i = 0; i < capacity; i++) {
            if (!spotsOccupied[i]) return i;
        }
        return -1;
    }

    void occupySpot(int idx) {
        if (idx >= 0 && idx < capacity) spotsOccupied[idx] = true;
    }

    void freeSpot(int idx) {
        if (idx >= 0 && idx < capacity) spotsOccupied[idx] = false;
    }
};