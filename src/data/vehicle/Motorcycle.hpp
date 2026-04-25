#pragma once

#include <memory>

#include "Vehicle.hpp"

// ====

struct MotorcycleData {
    VehicleData base;
    bool hasSidecar = false;
};

class Motorcycle: public Vehicle {
private:
    bool m_hasSidecar;

public:
    Motorcycle(MotorcycleData data) : Vehicle(std::move(data.base)), m_hasSidecar(data.hasSidecar) {}

    static std::unique_ptr<Motorcycle> fromJSON(const json& j);

    void printInfo() const override;


}; // Motorcycle
