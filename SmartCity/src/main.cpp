#include "../include/Simulation.hpp"
#include "../include/ParkingLogic.hpp"
#include "../include/CarLogic.hpp"
#include "../include/Utils.hpp"

#include <vector>
#include <string>
#include <cmath>

// ---------------------------
//  Données fiche parkings
// ---------------------------
struct ParkingInfo {
    std::string name;
    float price;
    std::string side;
    Color color;
};

static std::string GetCardinalSide(float x, float y, float centerX, float centerY) {
    float dx = x - centerX;
    float dy = y - centerY;
    if (std::fabs(dx) > std::fabs(dy)) return (dx > 0) ? "Est" : "Ouest";
    return (dy > 0) ? "Sud" : "Nord";
}

// ---------------------------
//  INTRO : image plein écran
// ---------------------------
static void DrawIntroFullScreen(Texture2D img, int sw, int sh) {
    ClearBackground(BLACK);

    if (img.id != 0) {
        DrawTexturePro(
            img,
            Rectangle{0, 0, (float)img.width, (float)img.height},
            Rectangle{0, 0, (float)sw, (float)sh},
            Vector2{0, 0},
            0.0f,
            WHITE
        );
    } else {
        DrawText("Image intro non chargee", 20, 20, 24, RED);
    }
}

// ---------------------------
//  FICHE : tableau + bouton
// ---------------------------
static void DrawInfoSheet(const std::vector<ParkingInfo>& infos, int sw, int sh, bool hoveringBtn) {
    // Fond black de la page fiche
    ClearBackground(BLACK);

    const int tableW = 520;
    const int tableH = 260;
    const int tx = (sw - tableW) / 2;
    const int ty = (sh - tableH) / 2;

    Color bordeaux = {128, 0, 32, 255};
    DrawRectangle(tx, ty, tableW, tableH, bordeaux);
    DrawRectangleLines(tx, ty, tableW, tableH, WHITE);

    const char* title = "Informations sur les parkings";
    int titleFS = 28;
    int titleW = MeasureText(title, titleFS);
    DrawText(title, tx + (tableW - titleW)/2, ty + 18, titleFS, RAYWHITE);

    // colonnes
    int colNameX  = tx + 25;
    int colPriceX = tx + 185;
    int colSideX  = tx + 360;

    int startY = ty + 70;
    int lineH = 40;

    for (size_t i = 0; i < infos.size(); ++i) {
        const auto& p = infos[i];
        int y = startY + (int)i * lineH;
        DrawText(p.name.c_str(), colNameX, y, 22, p.color);
        DrawText(TextFormat("Prix : %.2f dh/h", p.price), colPriceX, y, 22, RAYWHITE);
        DrawText(("Cote : " + p.side).c_str(), colSideX, y, 22, RAYWHITE);
    }

    // bouton démarrer sous le tableau
    const int btnW = 140;
    const int btnH = 42;
    const int btnX = tx + (tableW - btnW)/2;
    const int btnY = ty + tableH + 20;

    Color btnColor = hoveringBtn ? Fade(GREEN, 0.85f) : GREEN;
    DrawRectangle(btnX, btnY, btnW, btnH, btnColor);
    DrawRectangleLines(btnX, btnY, btnW, btnH, Fade(WHITE, 0.8f));

    const char* btnText = "Demarrer";
    int btnFS = 22;
    int btnTW = MeasureText(btnText, btnFS);
    DrawText(btnText, btnX + (btnW - btnTW)/2, btnY + (btnH - btnFS)/2, btnFS, WHITE);
}

// ---------------------------
//  AUDIO UI : slider + muet
// ---------------------------
static float Clamp01(float v) {
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

static void UpdateAudioUI(Rectangle panel, float& volume, bool& muted, float& volumeBeforeMute) {
    Vector2 m = GetMousePosition();

    int pad = 10;
    float sliderX = panel.x + pad + 85.0f;
    float sliderY = panel.y + 38.0f;
    float sliderW = panel.width - (pad * 2.0f) - 85.0f - 90.0f;
    float sliderH = 10.0f;

    Rectangle slider{sliderX, sliderY, sliderW, sliderH};
    Rectangle muteBtn{slider.x + slider.width + 12.0f, slider.y - 8.0f, 78.0f, 26.0f};

    // clic muet
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(m, muteBtn)) {
        muted = !muted;
        if (muted) volumeBeforeMute = volume;
        else volume = volumeBeforeMute;
    }

    // drag slider
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(m, slider)) {
        float t = (m.x - slider.x) / slider.width;
        volume = Clamp01(t);
        if (volume <= 0.001f) muted = true;
        else {
            muted = false;
            volumeBeforeMute = volume;
        }
    }
}

static void DrawAudioUI(Rectangle panel, float volume, bool muted) {
    DrawRectangleRec(panel, Fade(BLACK, 0.35f));
    DrawRectangleLinesEx(panel, 2, Fade(WHITE, 0.7f));

    int pad = 10;
    DrawText("Audio", (int)panel.x + pad, (int)panel.y + pad, 18, WHITE);
    DrawText("Volume", (int)panel.x + pad, (int)panel.y + 36, 16, WHITE);

    float sliderX = panel.x + pad + 85.0f;
    float sliderY = panel.y + 40.0f;
    float sliderW = panel.width - (pad * 2.0f) - 85.0f - 90.0f;
    float sliderH = 10.0f;

    Rectangle slider{sliderX, sliderY, sliderW, sliderH};
    DrawRectangleRec(slider, Fade(WHITE, 0.20f));
    DrawRectangleLinesEx(slider, 1, Fade(WHITE, 0.6f));

    float t = muted ? 0.0f : volume;
    Rectangle fill = slider;
    fill.width = slider.width * t;
    DrawRectangleRec(fill, Fade(GREEN, 0.85f));

    float knobX = slider.x + slider.width * t;
    DrawCircleV(Vector2{knobX, slider.y + slider.height/2}, 7.0f, RAYWHITE);

    Rectangle muteBtn{slider.x + slider.width + 12.0f, slider.y - 8.0f, 78.0f, 26.0f};
    DrawRectangleRec(muteBtn, muted ? MAROON : DARKGREEN);
    DrawRectangleLinesEx(muteBtn, 1, Fade(WHITE, 0.8f));
    DrawText(muted ? "Muet" : "Son", (int)muteBtn.x + 18, (int)muteBtn.y + 5, 16, WHITE);
}

// ---------------------------
//  DASHBOARD occupation parkings (simulation)
// ---------------------------
static int CountOccupiedSpots(const ParkingLot& p) {
    int occ = 0;
    for (bool b : p.spotsOccupied) if (b) occ++;
    return occ;
}

static void DrawParkingDashboard(const std::vector<ParkingLot>& parkings, int screenW) {
    const int panelX = 10;
    const int panelY = 10;
    const int panelW = screenW - 20;
    const int panelH = 70;

    DrawRectangle(panelX, panelY, panelW, panelH, Fade(BLACK, 0.35f));
    DrawRectangleLines(panelX, panelY, panelW, panelH, Fade(WHITE, 0.7f));
    DrawText("Occupation des parkings", panelX + 12, panelY + 8, 18, RAYWHITE);

    int x = panelX + 12;
    int y = panelY + 32;

    for (const auto& p : parkings) {
        int total = p.capacity;
        int occ   = CountOccupiedSpots(p);
        float ratio = (total > 0) ? (float)occ / (float)total : 0.0f;

        std::string label = std::string(p.name) + " : " + std::to_string(occ) + "/" + std::to_string(total);
        DrawText(label.c_str(), x, y, 16, RAYWHITE);

        Rectangle bg = { (float)x, (float)(y + 18), 140.0f, 10.0f };
        DrawRectangleRec(bg, Fade(WHITE, 0.20f));
        DrawRectangleLinesEx(bg, 1, Fade(WHITE, 0.5f));

        Rectangle fill = bg;
        fill.width = bg.width * ratio;

        Color c = (ratio < 0.7f) ? GREEN : (ratio < 0.9f) ? ORANGE : RED;
        DrawRectangleRec(fill, Fade(c, 0.85f));

        x += 210;
    }
}

// ---------------------------
//  SPEED UI : slider
// ---------------------------
static void UpdateSpeedControl(Rectangle panel, float& timeScale) {
    Vector2 m = GetMousePosition();

    int pad = 10;
    float sliderX = panel.x + pad + 85.0f;
    float sliderY = panel.y + 15.0f; // Centré verticalement
    float sliderW = panel.width - (pad * 2.0f) - 85.0f - 20.0f;
    float sliderH = 10.0f;

    Rectangle slider{sliderX, sliderY, sliderW, sliderH};

    // Drag slider
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        // Zone de détection un peu plus large pour le confort
        Rectangle touchZone = slider;
        touchZone.y -= 10;
        touchZone.height += 20;
        
        if (CheckCollisionPointRec(m, touchZone)) {
            float t = (m.x - slider.x) / slider.width;
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;
            
            // Map 0.0-1.0 to 0.0x-3.0x speed
            timeScale = t * 3.0f;
        }
    }
}

static void DrawSpeedControl(Rectangle panel, float timeScale) {
    DrawRectangleRec(panel, Fade(BLACK, 0.35f));
    DrawRectangleLinesEx(panel, 2, Fade(WHITE, 0.7f));

    int pad = 10;
    DrawText("Vitesse", (int)panel.x + pad, (int)panel.y + 12, 18, WHITE);

    float sliderX = panel.x + pad + 85.0f;
    float sliderY = panel.y + 15.0f;
    float sliderW = panel.width - (pad * 2.0f) - 85.0f - 20.0f;
    float sliderH = 10.0f;

    Rectangle slider{sliderX, sliderY, sliderW, sliderH};
    DrawRectangleRec(slider, Fade(WHITE, 0.20f));
    DrawRectangleLinesEx(slider, 1, Fade(WHITE, 0.6f));

    float t = timeScale / 3.0f; // Normalize back to 0-1
    if (t > 1.0f) t = 1.0f;

    Rectangle fill = slider;
    fill.width = slider.width * t;
    DrawRectangleRec(fill, Fade(BLUE, 0.85f));

    float knobX = slider.x + slider.width * t;
    DrawCircleV(Vector2{knobX, slider.y + slider.height/2}, 7.0f, RAYWHITE);
    
    // Affichage valeur x1.0, x2.5 etc.
    DrawText(TextFormat("x%.1f", timeScale), (int)(sliderX + sliderW + 5), (int)sliderY - 2, 12, WHITE);
}

// ---------------------------
//              MAIN
// ---------------------------
enum class ScreenState { INTRO, INFO, SIM };

int main() {
    // la taille de la fenetre raylib 
    const int screenW = 1200;
    const int screenH = 900;

    InitWindow(screenW, screenH, "Smart City");
    SetTargetFPS(60);

    // --- Intro image ---
    Texture2D introImage = LoadTexture("assets/accueil.png");
    if (introImage.id == 0) {
        TraceLog(LOG_ERROR, "Impossible de charger assets/accueil.png (chemin relatif au dossier d'execution).");
    }

    // --- Audio ---
    InitAudioDevice();
    Music menuMusic = LoadMusicStream("assets/menu.mp3");
    bool musicOk = (menuMusic.stream.buffer != nullptr); 

    if (musicOk) {
        PlayMusicStream(menuMusic);
    } else {
        TraceLog(LOG_ERROR, "Impossible de charger assets/accueil.png");
    }

    float volume = 0.6f;
    float volumeBeforeMute = volume;
    bool muted = false;
    bool musicPlaying = false;

    // --- Infos parkings pour la fiche ---
    Vector2 cityCenter = {screenW/2.f, screenH/2.f};

    std::vector<std::pair<std::string, Vector2>> parkingPositions = {
        {"VIP",     {100,  50}},
        {"Central", {400, 350}},
        {"Eco",     { 50, 630}},
        {"City",    {600, 630}}
    };

    std::vector<float> parkingPrices = {15.0f, 8.0f, 2.0f, 5.0f};
    std::vector<Color> parkingColors = {WHITE, PURPLE, GREEN, ORANGE};

    std::vector<ParkingInfo> parkingInfos;
    for (size_t i = 0; i < parkingPositions.size(); ++i) {
        std::string side = GetCardinalSide(
            parkingPositions[i].second.x, parkingPositions[i].second.y,
            cityCenter.x, cityCenter.y
        );
        parkingInfos.push_back({parkingPositions[i].first, parkingPrices[i], side, parkingColors[i]});
    }

    // --- Navigation pages ---
    ScreenState state = ScreenState::INTRO;
    bool startSimulation = false;

    // ---------------------------
    //  BOUCLE UI : INTRO -> INFO
    // ---------------------------
    while (!WindowShouldClose() && !startSimulation) {
        Vector2 mousePos = GetMousePosition();

        if (state == ScreenState::INFO && musicOk) {
            if (!musicPlaying) {
                PlayMusicStream(menuMusic);
                musicPlaying = true;
            }
            UpdateMusicStream(menuMusic);
            SetMusicVolume(menuMusic, muted ? 0.0f : volume);
        }

        BeginDrawing();

        if (state == ScreenState::INTRO) {
            DrawIntroFullScreen(introImage, screenW, screenH);
            if (IsKeyPressed(KEY_SPACE)) state = ScreenState::INFO;
        }
        else if (state == ScreenState::INFO) {
            const int tableW = 520;
            const int tableH = 260;
            const int tx = (screenW - tableW) / 2;
            const int ty = (screenH - tableH) / 2;

            Rectangle startBtn = {
                (float)(tx + (tableW - 140)/2),
                (float)(ty + tableH + 20),
                140.0f,
                42.0f
            };
            bool hovering = CheckCollisionPointRec(mousePos, startBtn);

            DrawInfoSheet(parkingInfos, screenW, screenH, hovering);

            Rectangle audioPanel = { 20, 20, 360, 80 };
            UpdateAudioUI(audioPanel, volume, muted, volumeBeforeMute);
            DrawAudioUI(audioPanel, volume, muted);

            if ((hovering && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) || IsKeyPressed(KEY_SPACE)) {
                startSimulation = true;
            }
        }

        EndDrawing();
    }

    // Si l'utilisateur ferme la fenêtre pendant l'intro, on quitte proprement
    if (WindowShouldClose()) {
        if (musicOk) { StopMusicStream(menuMusic); UnloadMusicStream(menuMusic); }
        CloseAudioDevice();
        if (introImage.id != 0) UnloadTexture(introImage);
        CloseWindow();
        return 0;
    }

    // ---------------------------
    // SIMULATION : initialisation
    // ---------------------------
    std::vector<Road> roads;
    Road r1 = {{-100, 250}, {1300, 250}, 2, 80.0f};
    r1.light = {r1.end, LIGHT_GREEN, 5.0f};
    Road r2 = {{1300, 600}, {-100, 600}, 2, 80.0f};
    r2.light = {r2.end, LIGHT_RED, 5.0f};
    roads.push_back(r1);
    roads.push_back(r2);

    std::vector<ParkingLot> parkings;
    
parkings.push_back(ParkingLot({100, 70}, {150, 80}, 4, 15.0f, "VIP", BLUE, {175, 250}));
parkings.push_back(ParkingLot({450, 425}, {200, 80}, 6, 8.0f, "Central", PURPLE, {650, 290}));

// --- Road 2 (Y = 600) ---
// 600 (Road) + 100 (Espace) = 700
parkings.push_back(ParkingLot({100, 700}, {180, 80}, 5, 2.0f, "Eco", GREEN, {200, 600}));
parkings.push_back(ParkingLot({750, 700}, {250, 80}, 7, 5.0f, "City", ORANGE, {870, 600}));
    std::vector<Car> cars;
    const int nbCars = 20;
    for (int i = 0; i < nbCars; i++) {
        Car c;
        c.id = i;
        c.roadIndex = (i < nbCars/2) ? 0 : 1;
        c.currentLane = GetRandomValue(0, 1);
        c.targetLane = c.currentLane;
        c.distance = (i % (nbCars/2)) * 100.f;
        c.speed = MAX_SPEED;
        c.color = (i % 3 == 0) ? RED : (i % 3 == 1) ? BLUE : DARKGREEN;
        c.laneOffset = (c.currentLane == 0) ? -roads[c.roadIndex].width/4 : roads[c.roadIndex].width/4;
        c.laneChangeTimer = 0;
        c.state = DRIVING;
        c.parkingIdx = -1;
        c.spotIdx = -1;
        c.worldPos = {0,0};
        c.targetPos = {0,0};
        if (i == 2 || i == 8) { c.color = ORANGE; c.speed = 60.f; }
        cars.push_back(c);
    }

    float simulationTime = 0.0f; 
    float timeScale = 1.0f; // Vitesse par défaut

    // ---------------------------
    // Boucle principale simulation
    // ---------------------------
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        simulationTime += dt * timeScale; // Le temps affiché suit la vitesse

        // Mise à jour logique avec le timeScale
        UpdateTraffic(cars, roads, parkings, dt * timeScale);
        
        // Musique de fond continue
        if (musicOk) {
            UpdateMusicStream(menuMusic); 
        }

        // Dessin
        BeginDrawing();
        ClearBackground(GetColor(0x228B22FF)); // herbe
        
        // Dessin des allées
        DrawDriveway(parkings[0], roads[0]);
        DrawDriveway(parkings[1], roads[0]);
        DrawDriveway(parkings[2], roads[1]);
        DrawDriveway(parkings[3], roads[1]);

        for (const auto& p : parkings) DrawParking(p);

        // Routes + lignes d'arrêt + feux
        for (const auto& road : roads) {
            DrawLineEx(road.start, road.end, road.width + 12, GRAY);
            DrawLineEx(road.start, road.end, road.width, GetColor(0x333333FF));
            DrawDashedLine(road.start, road.end, 2.0f, YELLOW);

            Vector2 dir = road.getDir();
            Vector2 normal = { -dir.y, dir.x };

            // Ligne d'arrêt
            float STOP_OFFSET = 200.0f;
            Vector2 stopLineCenter = Vector2Subtract(road.end, Vector2Scale(dir, STOP_OFFSET));

            Vector2 stopA = Vector2Add(stopLineCenter, Vector2Scale(normal,  road.width / 2));
            Vector2 stopB = Vector2Add(stopLineCenter, Vector2Scale(normal, -road.width / 2));
            DrawLineEx(stopA, stopB, 4.0f, WHITE);

            // Feu tricolore
            float LIGHT_SHIFT_X = 60.0f;
            Vector2 lightPos = Vector2Add(stopLineCenter, Vector2Scale(normal, road.width/2 + 30));
            lightPos.x += LIGHT_SHIFT_X;

            DrawLineEx(Vector2Add(stopLineCenter, Vector2Scale(normal, road.width/2)),
                       lightPos, 4.0f, DARKGRAY);
            DrawRectangle((int)lightPos.x - 12, (int)lightPos.y - 35, 24, 70, BLACK);

            DrawCircle((int)lightPos.x, (int)lightPos.y - 22, 9,
                       (road.light.state == LIGHT_RED) ? RED : Fade(RED, 0.2f));
            DrawCircle((int)lightPos.x, (int)lightPos.y, 9,
                       (road.light.state == LIGHT_YELLOW) ? YELLOW : Fade(YELLOW, 0.2f));
            DrawCircle((int)lightPos.x, (int)lightPos.y + 22, 9,
                       (road.light.state == LIGHT_GREEN) ? GREEN : Fade(GREEN, 0.2f));
        }

        // Voitures
        for (const auto& car : cars) DrawCar(car, roads[car.roadIndex]);

        // Dashboard occupation
        DrawParkingDashboard(parkings, screenW);

        // Speed Control UI 
        Rectangle speedPanel = { screenW - 350.0f, 150.0f, 330.0f, 40.0f };
        UpdateSpeedControl(speedPanel, timeScale);
        DrawSpeedControl(speedPanel, timeScale);

        // Timer
        int m = (int)simulationTime / 60; 
        int s = (int)simulationTime % 60;
        DrawRectangle(screenW - 160, 90, 140, 50, Fade(BLACK, 0.6f));
        DrawRectangleLines(screenW - 160, 90, 140, 50, WHITE);
        DrawText(TextFormat("%02d:%02d", m, s), screenW - 145, 100, 30, GREEN);

        EndDrawing();
    }

    // ---------------------------
    // NETTOYAGE (Seulement à la fin)
    // ---------------------------
    if (musicOk) {
        StopMusicStream(menuMusic);
        UnloadMusicStream(menuMusic);
    }
    CloseAudioDevice();

    if (introImage.id != 0) UnloadTexture(introImage);

    CloseWindow();

    return 0;
}