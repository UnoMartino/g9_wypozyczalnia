#pragma once

#include <memory>
#include <vector>

#include "data/vehicle/Vehicle.hpp"

// ====

// stores application state data
struct ApplicationState {
    std::vector<std::unique_ptr<Vehicle>> loadedVehicles;       // store vehicle list from 'data.json'

    bool isRunning = true;                                      // is application running
}; // ApplicationState

// ====

class Application {
private:
    ApplicationState m_state;

    std::unique_ptr<Vehicle> parseVehicle(const json& j);

public:
    Application() {}
    ~Application() {}

    void run();
    void shutdown();

    void debugPrintVehicles() const;
}; // Application
