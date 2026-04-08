#include "Application.hpp"

#include "data/Loader.hpp"

#include "data/vehicle/Vehicle.hpp"
#include "data/vehicle/Car.hpp"
#include "data/vehicle/Motorcycle.hpp"
#include "data/vehicle/Truck.hpp"


#include <iostream>

// ====

// load the application state and data
void Application::run() {

    json data = loadFile("./data.json");
    for (const auto& item : data) {
        if (auto vehicle = parseVehicle(item)) {
            m_state.loadedVehicles.push_back(std::move(vehicle));
        }
    }

    debugPrintVehicles();

    while (m_state.isRunning) {

    }

} // Application::run

std::unique_ptr<Vehicle> Application::parseVehicle(const json& item) {
    if (!item.contains("kind")) {
        std::cerr << "Missing kind\n";
        return nullptr;
    }

    auto kind = item.at("kind").get<VehicleKind>();

    switch (kind) {
        case VehicleKind::Car: { return Car::fromJSON(item); }
        case VehicleKind::Motorcycle: { return Motorcycle::fromJSON(item); }
        case VehicleKind::Truck: { return Truck::fromJSON(item); }

        default: return nullptr;
    }
} // Application::parseVehicle

// saves the application state and data
// prepares to exit the application
void Application::shutdown() {
    m_state.isRunning = false;
} // Application::shutdown


// ==== DEBUG

void Application::debugPrintVehicles() const {
    std::cout << "\n [DEBUG] Loaded vehicles:\n";

    for (const auto& vehicle : m_state.loadedVehicles) {
        if (vehicle) {
            vehicle->printInfo();
            std::cout << "----------------------------------------\n";
        }
    }
} // Application::debugPrintVehicles
