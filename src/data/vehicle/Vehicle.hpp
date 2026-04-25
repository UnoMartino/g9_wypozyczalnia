#pragma once

#include <optional>
#include <stdint.h>
#include <string>

#include "../../../ext/json.hpp"

// ====

using json = nlohmann::json;

enum class VehicleTier { Premium = 1, Standard, Economy, Budget, Basic, Utility };
enum class VehicleKind { Car, Motorcycle, Truck };

// ====

struct VehicleData {
    int id = -1;                            // default: uninitialized
    VehicleTier tier;
    VehicleKind kind;

    std::string modelName = "unknown";
    std::optional<std::string> licensePlate = std::nullopt;

    uint32_t mileage = 0;
    bool needsService = false;

    int64_t dueDate;                        // final rent day
    uint32_t price = 0;
}; // VehicleData

class Vehicle {
protected:
    VehicleData m_commonData;

public:

    Vehicle(VehicleData commonData) : m_commonData(std::move(commonData)) {}
    virtual ~Vehicle() = default;

    virtual void printInfo() const = 0;

private:

}; // Vehicle
