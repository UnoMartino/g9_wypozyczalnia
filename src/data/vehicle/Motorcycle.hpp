#pragma once

#include <memory>

#include "Vehicle.hpp"

// ====

struct MotorcycleData {
    VehicleData base;
    bool hasSidecar = false;
    bool requiresFullLicense = false;
    bool hasLuggagePanniers = false;
};

class Motorcycle: public Vehicle {
private:
    bool m_hasSidecar = false;
    bool m_requiresFullLicense = false;
    bool m_hasLuggagePanniers = false;
public:
    Motorcycle(MotorcycleData data) : Vehicle(std::move(data.base)), m_hasSidecar(data.hasSidecar),
        m_requiresFullLicense(data.requiresFullLicense), m_hasLuggagePanniers(data.hasLuggagePanniers) {}

    static std::unique_ptr<Motorcycle> fromJSON(const json& j);

    void printInfo() const override;

    std::map<std::string, std::string> getDetails() const override {
        return {
            {"Wózek", m_hasSidecar ? "Posiada" : "Nie posiada"},
            {"Prawo jazdy kat. A", m_requiresFullLicense ? "Wymaga" : "Nie wymaga"},
            {"Schowki", m_hasLuggagePanniers ? "Posiada" : "Nie posiada"},
        };
    };

    json toJSON() const override;

}; // Motorcycle
