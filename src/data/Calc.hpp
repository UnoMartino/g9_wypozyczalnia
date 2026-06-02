#pragma once

// ====

#include "vehicle/Vehicle.hpp"

static double calculateRentCost(int mainPrice, int days, VehicleTier tier) {

    double multiplier = 1.0;

    switch (tier) {
        case VehicleTier::Premium:
            multiplier = 1.2;
            break;
        case VehicleTier::Standard:
            multiplier = 1.1;
            break;
        case VehicleTier::Economy:
            multiplier = 1.05;
            break;
        case VehicleTier::Budget:
            multiplier = 1.0;
            break;
        case VehicleTier::Basic:
            multiplier = 0.95;
            break;
        case VehicleTier::Utility:
            multiplier = 0.9;
            break;
    }

    // the more days present the lower prices should be
    double durationDiscount = 1.0; // No discount for 1-2 days

    if (days >= 21) {
        durationDiscount = 0.80; // 20% off for 3+ weeks
    } else if (days >= 7) {
        durationDiscount = 0.85; // 15% off for 1+ week
    } else if (days >= 3) {
        durationDiscount = 0.90; // 10% off for 3+ days
    }

    return mainPrice * days * multiplier * durationDiscount;
}

static double calculateMileageLimitCost(int option, int days) {

    switch (option) {
        case 0: return 0.0;
        case 1: return 20.0 * days;
        case 2: return 50.0 * days;
    }

    return 0.0;
}
