#include "Truck.hpp"

#include <iostream>


// ====

std::unique_ptr<Truck> Truck::fromJSON(const json& j) {
    TruckData data;

    data.base.id = j.at("id").get<int>();
    data.base.tier = j.at("tier").get<VehicleTier>();
    data.base.kind = VehicleKind::Truck;
    data.base.modelName = j.at("modelName").get<std::string>();

    if (j.contains("licensePlate") && !j["licensePlate"].is_null()) {
        data.base.licensePlate = j.at("licensePlate").get<std::string>();
    }

    data.base.mileage = j.at("mileage").get<uint32_t>();
    data.base.needsService = j.at("needsService").get<bool>();
    data.base.dueDate = j.at("dueDate").get<int64_t>();
    data.base.price = j.at("price").get<uint32_t>();

    data.maxPayloadKg = j.at("maxPayloadKg").get<int>();

    return std::make_unique<Truck>(std::move(data));
} // Truck::fromJSON


void Truck::printInfo() const {
    std::cout << "[TRUCK] ID: " << m_commonData.id
              << " | Model: " << m_commonData.modelName << "\n";

    std::cout << "Rejestracja: " << m_commonData.licensePlate.value_or("BRAK") << "\n";

    std::cout << "Przebieg: " << m_commonData.mileage << " km\n"
              << "Wymaga serwisu: " << (m_commonData.needsService ? "TAK" : "NIE") << "\n"
              << "Maksymalna waga: " << m_maxPayloadKg << " kg\n";
} // Truck::printInfo
