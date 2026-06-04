#pragma once

#include <memory>

#include "Vehicle.hpp"

// ====

enum TrailerType {
    Refrigerated,
    Tank,
    Dry,
};

NLOHMANN_JSON_SERIALIZE_ENUM(TrailerType, {
    {Dry, nullptr},
    {Refrigerated, 0},
    {Tank, 1},
    {Dry, 2}
})

struct TruckData {
    VehicleData base;
    uint32_t maxPayloadKg = 0;

    TrailerType trailerType;

    bool hasSleeperCab = false;
};

class Truck: public Vehicle {
private:
    uint32_t m_maxPayloadKg = 0;
    bool m_hasSleeperCab = false;

    std::string m_trailerType = "Unknown";

    std::string typeToString(TrailerType t) const {
        switch (t) {
            case Refrigerated: return "Chłodnia";
            case Tank: return "Cysterna";
            case Dry: return "TIR";
            default: return "BŁĄD";
        }
    }

public:
    Truck(TruckData data) : Vehicle(std::move(data.base)), m_maxPayloadKg(data.maxPayloadKg),
    m_hasSleeperCab(data.hasSleeperCab), m_trailerType(typeToString(data.trailerType)) {}

    static std::unique_ptr<Truck> fromJSON(const json& j);

    void printInfo() const override;

    std::map<std::string, std::string> getDetails() const override {
        return {
            {"Typ naczepy", m_trailerType},
            {"Maksymalna masa", std::to_string(m_maxPayloadKg) + "kg"},
            {"Kabina sypialniana", m_hasSleeperCab ? "Posiada" : "Nie posiada"},
        };
    };

}; // Truck
