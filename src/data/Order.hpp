#pragma once

#include <string>
#include <vector>

#include "../ext/json.hpp"

#include "Util.hpp"

// ====

struct Order {

    // util
    int vehicleId = 0;
    DateRange rentRange;  // days

    // form

    std::string firstName;
    std::string lastName;

    std::string email;

    bool wantsInsurance = false;

    int mileageTier = 0; // 0 = Basic, 1 = Extended, 2 = Unlimited
    std::vector<std::string> mileageOptions = {
        "100 km/dzień (W cenie)",
        "300 km/dzień (+20 PLN/dzień)",
        "Bez limitu (+50 PLN/dzień)"
    };

    double price = 0;
};

inline void to_json(nlohmann::json& j, const Order& o) {
    j = nlohmann::json{
        {"vehicleId", o.vehicleId},
        {"rentRange", o.rentRange},
        {"firstName", o.firstName},
        {"lastName", o.lastName},
        {"email", o.email},
        {"wantsInsurance", o.wantsInsurance},
        {"mileageTier", o.mileageTier},
        {"price", o.price}
    };
}

inline void from_json(const nlohmann::json& j, Order& o) {
    j.at("vehicleId").get_to(o.vehicleId);
    j.at("rentRange").get_to(o.rentRange);
    j.at("firstName").get_to(o.firstName);
    j.at("lastName").get_to(o.lastName);
    j.at("email").get_to(o.email);
    j.at("wantsInsurance").get_to(o.wantsInsurance);
    j.at("mileageTier").get_to(o.mileageTier);
    j.at("price").get_to(o.price);
}

void saveOrders(const std::vector<Order>& orders);
