#include "Vehicle.hpp"
#include <fstream>

// ====

void saveVehicles(const std::vector<std::unique_ptr<Vehicle>>& vehicles) {
    json j = json::array();
    for (const auto& v : vehicles) {
        if (v) {
            j.push_back(v->toJSON());
        }
    }
    std::ofstream file("./data.json");
    if (file.is_open()) {
        file << j.dump(4);
    }
}
