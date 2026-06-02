#pragma once

#include <memory>

#include "Vehicle.hpp"

// ====

enum BodyStyle {
    Sedan, SUV, StateWagon, Hatchback, None
};

struct CarData {
    VehicleData base;

    int trunkCapacityLiters = 0;

    BodyStyle bodyStyle = None;

    bool isElectric = false;
}; // CarData

class Car: public Vehicle {
private:

    int m_trunkCapacityLiters = 0;

    std::string m_bodyStyle = "Unknown";

    bool m_isElectric = false;

    std::string styleToString(BodyStyle s) const {
        switch (s) {
            case Sedan: return "Sedan";
            case SUV: return "SUV";
            case StateWagon: return "Kombi";
            case Hatchback: return "Hatchback";
            default: return "Unknown";
        }
    }

public:
    Car(CarData data) : Vehicle(std::move(data.base)),
    m_trunkCapacityLiters(data.trunkCapacityLiters),
    m_bodyStyle(styleToString(data.bodyStyle)),
    m_isElectric(data.isElectric)
    {}

    static std::unique_ptr<Car> fromJSON(const json& j);

    void printInfo() const override;

    std::map<std::string, std::string> getDetails() const override {
        return {
            {"Nadwozie", m_bodyStyle},
            {"Pojemność bagażnika", std::to_string(m_trunkCapacityLiters) + " L"},
            {"Pojazd elektryczny", m_isElectric ? "Tak" : "Nie"},
        };
    };

}; // Car
