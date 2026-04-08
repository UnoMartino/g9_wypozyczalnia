#pragma once

#include <memory>

#include "Vehicle.hpp"

// ====

struct TruckData {
    VehicleData base;
    uint32_t maxPayloadKg = 0;
};

class Truck: public Vehicle {
private:
    uint32_t m_maxPayloadKg;

public:
    Truck(TruckData data) : Vehicle(std::move(data.base)), m_maxPayloadKg(data.maxPayloadKg) {}

    static std::unique_ptr<Truck> fromJSON(const json& j);

    void printInfo() const override;


}; // Truck
