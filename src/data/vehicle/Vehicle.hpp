#pragma once

#include <optional>
#include <stdint.h>
#include <string>

#include "../../../ext/json.hpp"

// ====

using json = nlohmann::json;

enum class VehicleTier { Premium = 1, Standard, Economy, Budget, Basic, Utility };
enum class VehicleKind { Car, Motorcycle, Truck };

static inline std::string tierToString(VehicleTier t) {
    switch (t) {
        case VehicleTier::Premium: return "Premium";
        case VehicleTier::Standard: return "Standard";
        case VehicleTier::Economy: return "Economy";
        case VehicleTier::Budget: return "Budget";
        case VehicleTier::Basic: return "Basic";
        case VehicleTier::Utility: return "Utility";
    }
}

// ====

struct VehicleData {
    int id = -1;                            // default: uninitialized
    VehicleTier tier;
    VehicleKind kind;

    std::string modelName = "unknown";
    std::optional<std::string> licensePlate = std::nullopt;

    uint32_t mileage = 0;
    bool needsService = false;

    uint32_t price = 0;

    int passengerCapacity = 0;
}; // VehicleData

class Vehicle {
protected:
    VehicleData m_commonData;

public:

    Vehicle(VehicleData commonData) : m_commonData(std::move(commonData)) {}
    virtual ~Vehicle() = default;

    virtual void printInfo() const = 0;
    virtual std::map<std::string, std::string> getDetails() const = 0;

    VehicleKind getKind() { return m_commonData.kind; }
    VehicleTier getTier() { return m_commonData.tier; }
    uint32_t getPrice() { return m_commonData.price; }
    std::string getName() { return m_commonData.modelName; }
    int getId() const { return m_commonData.id; }

    virtual json toJSON() const = 0;

protected:
    json getCommonJSON() const {
        json j = {
            {"id", m_commonData.id},
            {"tier", static_cast<int>(m_commonData.tier)},
            {"kind", static_cast<int>(m_commonData.kind)},
            {"modelName", m_commonData.modelName},
            {"mileage", m_commonData.mileage},
            {"needsService", m_commonData.needsService},
            {"price", m_commonData.price},
            {"passengerCapacity", m_commonData.passengerCapacity}
        };
        if (m_commonData.licensePlate) {
            j["licensePlate"] = *m_commonData.licensePlate;
        } else {
            j["licensePlate"] = nullptr;
        }
        return j;
    }

private:

}; // Vehicle

void saveVehicles(const std::vector<std::unique_ptr<Vehicle>>& vehicles);
