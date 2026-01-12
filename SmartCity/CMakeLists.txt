cmake_minimum_required(VERSION 3.14)

project(SmartCitySim VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# --- CHEMINS ---
set(RAYLIB_PATH "C:/raylib/raylib-5.5_win64_mingw-w64")

include_directories(${RAYLIB_PATH}/include)
include_directories(${CMAKE_SOURCE_DIR}/include)
link_directories(${RAYLIB_PATH}/lib)

# --- SIMULATION (Mode Fenêtre) ---
add_executable(SmartCitySim WIN32
    src/main.cpp
    src/Simulation.cpp
    src/ParkingLogic.cpp  
    src/CarLogic.cpp  
    src/Utils.cpp
    
)

#target_link_libraries(SmartCitySim raylib -lgdi32 -lwinmm)
target_link_libraries(SmartCitySim raylib gdi32 winmm user32 shell32)
# --- TESTS (Mode Console) ---
add_executable(TrafficTests
    tests/TestTraffic.cpp
    src/Simulation.cpp
    src/ParkingLogic.cpp
    src/CarLogic.cpp
    src/Utils.cpp
)

# FORCE LE MODE CONSOLE
set_target_properties(TrafficTests PROPERTIES WIN32_EXECUTABLE OFF)
target_link_options(TrafficTests PRIVATE -mconsole)

target_link_libraries(TrafficTests raylib -lgdi32 -lwinmm)