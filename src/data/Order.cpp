#include "Order.hpp"

#include <fstream>
#include <vector>

#include "../ext/json.hpp"

// ====

void saveOrders(const std::vector<Order>& orders) {
    std::ofstream file("./orders.json");

    if (!file.is_open()) return;

    nlohmann::json jsonData = nlohmann::json::array();

    for (auto& order : orders) {
        jsonData.push_back(nlohmann::json(order));
    }
    file << jsonData.dump(4);
}
