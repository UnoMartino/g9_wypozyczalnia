#include "Car.hpp"

#include <iostream>

// ====

std::unique_ptr<Car> Car::fromJSON(const json& j) {
    CarData data;

    data.base.id = j.at("id").get<int>();
    data.base.tier = j.at("tier").get<VehicleTier>();
    data.base.kind = VehicleKind::Car;
    data.base.modelName = j.at("modelName").get<std::string>();

    if (j.contains("licensePlate") && !j["licensePlate"].is_null()) {
        data.base.licensePlate = j.at("licensePlate").get<std::string>();
    }

    data.base.mileage = j.at("mileage").get<uint32_t>();
    data.base.needsService = j.at("needsService").get<bool>();
    data.base.dueDate = j.at("dueDate").get<int64_t>();
    data.base.price = j.at("price").get<uint32_t>();
    data.base.passengerCapacity = j.at("passengerCapacity").get<int>();

    if (j.contains("trunkCapacityLiters")) {
        data.trunkCapacityLiters = j.at("trunkCapacityLiters").get<int>();
    }
    if (j.contains("bodyStyle")) {
        data.bodyStyle = j.at("bodyStyle").get<BodyStyle>();
    }
    if (j.contains("isElectric")) {
        data.isElectric = j.at("isElectric").get<bool>();
    }

    //data.passengerCapacity = j.at("passengerCapacity").get<int>();

    return std::make_unique<Car>(std::move(data));
} // Car::fromJSON


void Car::printInfo() const {
    std::cout << "[CAR] ID: " << m_commonData.id
              << " | Model: " << m_commonData.modelName << "\n";

    std::cout << "Rejestracja: " << m_commonData.licensePlate.value_or("BRAK") << "\n";

    std::cout << "Przebieg: " << m_commonData.mileage << " km\n"
              << "Wymaga serwisu: " << (m_commonData.needsService ? "TAK" : "NIE") << "\n";
              //<< "Miejsca: " << m_passengerCapacity << " osob\n";

} // Car::printInfo
