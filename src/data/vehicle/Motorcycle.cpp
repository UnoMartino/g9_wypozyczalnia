#include "Motorcycle.hpp"

#include <iostream>

// ====

std::unique_ptr<Motorcycle> Motorcycle::fromJSON(const json& j) {
    MotorcycleData data;

    data.base.id = j.at("id").get<int>();
    data.base.tier = j.at("tier").get<VehicleTier>();
    data.base.kind = VehicleKind::Motorcycle;
    data.base.modelName = j.at("modelName").get<std::string>();

    if (j.contains("licensePlate") && !j["licensePlate"].is_null()) {
        data.base.licensePlate = j.at("licensePlate").get<std::string>();
    }

    data.base.mileage = j.at("mileage").get<uint32_t>();
    data.base.needsService = j.at("needsService").get<bool>();
    data.base.dueDate = j.at("dueDate").get<int64_t>();
    data.base.price = j.at("price").get<uint32_t>();

    data.hasSidecar = j.at("hasSidecar").get<int>();

    return std::make_unique<Motorcycle>(std::move(data));
} // Motorcycle::fromJSON


void Motorcycle::printInfo() const {
    std::cout << "[MOTORCYCLE] ID: " << m_commonData.id
              << " | Model: " << m_commonData.modelName << "\n";

    std::cout << "Rejestracja: " << m_commonData.licensePlate.value_or("BRAK") << "\n";

    std::cout << "Przebieg: " << m_commonData.mileage << " km\n"
              << "Wymaga serwisu: " << (m_commonData.needsService ? "TAK" : "NIE") << "\n"
              << "Miejsce: " << (m_hasSidecar ? "TAK" : "NIE") << "\n";
} // Motorcycle::printInfo
