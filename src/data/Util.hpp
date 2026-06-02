#pragma once

#include <chrono>
#include <string>
#include <iomanip>
#include <sstream>
#include <stdexcept>

#include "../ext/json.hpp"

// ====

struct DateRange {
    std::chrono::system_clock::time_point start;
    std::chrono::system_clock::time_point end;

    int duration() const {
        return (std::chrono::duration_cast<std::chrono::hours>(end - start).count() / 24) + 1;
    }
};

// Helper function to serialize time_point to string
inline std::string format_date(const std::chrono::system_clock::time_point& tp) {
    auto time_c = std::chrono::system_clock::to_time_t(tp);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_c), "%d-%m-%Y");
    return ss.str();
}

inline void to_json(nlohmann::json& j, const DateRange& dr) {
    j = nlohmann::json{
        {"start", format_date(dr.start)},
        {"end", format_date(dr.end)}
    };
}

// Helper function to deserialize string to time_point
inline std::chrono::system_clock::time_point parse_date(const std::string& date_str) {
    std::tm tm = {};
    std::istringstream ss(date_str);

    // Parse the string into the tm struct
    ss >> std::get_time(&tm, "%d-%m-%Y");

    if (ss.fail()) {
        throw std::runtime_error("Date parsing took an L. Check your format.");
    }

    // Convert tm to time_t (local time), then to time_point
    std::time_t time_c = std::mktime(&tm);
    return std::chrono::system_clock::from_time_t(time_c);
}

inline void from_json(const nlohmann::json& j, DateRange& dr) {
    dr.start = parse_date(j.at("start").get<std::string>());
    dr.end = parse_date(j.at("end").get<std::string>());
}
