#pragma once

#include <memory>

#include "Vehicle.hpp"

// ====

struct CarData {
    VehicleData base;
    int passengerCapacity = 0;
}; // CarData

class Car: public Vehicle {
private:

    int m_passengerCapacity;

public:
    Car(CarData data) : Vehicle(std::move(data.base)), m_passengerCapacity(data.passengerCapacity) {}

    static std::unique_ptr<Car> fromJSON(const json& j);

    void printInfo() const override;

}; // Car
