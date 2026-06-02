#pragma once

#include <string>

// ====

struct Order {

    // util
    int vehicleId = 0;
    int rentRange = 0;  // days


    // form

    std::string firstName;
    std::string lastName;

    std::string email;

    bool wantsInsurance = false;
    int mileageLimit = 0;

    // rent range (days)

    int price = 0;

};
