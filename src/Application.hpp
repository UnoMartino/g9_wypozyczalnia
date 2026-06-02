#pragma once

#include <chrono>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "data/Loader.hpp"
#include "data/vehicle/Vehicle.hpp"
#include "data/vehicle/Car.hpp"
#include "data/vehicle/Truck.hpp"
#include "data/vehicle/Motorcycle.hpp"

#include "ftxui/component/component_base.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "util/Auth.hpp"

#include "tui/View.hpp"

// ====

struct FtxuiEventHash {
    std::size_t operator()(const ftxui::Event& event) const noexcept {
        return std::hash<std::string>{}(event.input());
    }
};

using ShortcutMap = std::unordered_map<ftxui::Event, std::function<void()>, FtxuiEventHash>;
using DateRange = std::pair<std::chrono::system_clock::time_point, std::chrono::system_clock::time_point>;

// ====

enum class FocusKind {
    TOPBAR,
    HOME,
    VEHICLE_DETAILS,
    VEHICLE_FORM,
    ADMIN_DASHBOARD,
    RIGHTBAR,
};

enum NavigationContextKind {
    NONE,
    HOME,
    VEHICLE_DETAILS,
    VEHICLE_FORM,
    ADMIN_DASHBOARD,
};

struct NavigationNode {
    NavigationContextKind contextId;
    std::string label;
};


struct CalendarState {
    int year;
    int month;
};

static std::vector<std::unique_ptr<Vehicle>> loadVehicles();
// stores application state data
struct ApplicationState {
    std::vector<std::unique_ptr<Vehicle>> loadedVehicles = loadVehicles();       // store vehicle list from 'data.json'

    bool isRunning = true;                                      // is application running

    bool isSignedIn = false;
    std::string signedInUser = "guest";

    // view management
    ScreenInteractive screen = ScreenInteractive::Fullscreen();

    std::vector<NavigationNode> navigationStack = {NavigationNode{HOME, "Home"}};
    FocusKind currentFocus = FocusKind::HOME;

    NavigationNode getCurrentContext() { return navigationStack.back(); }

    // time management
    CalendarState currentCalendarState = initSystemCalendar();
    CalendarState systemCalendarState = initSystemCalendar();

    std::optional<std::chrono::system_clock::time_point> rangeStart;
    std::optional<std::chrono::system_clock::time_point> rangeEnd;

    //

    CalendarState initSystemCalendar() {

        CalendarState s;

        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::tm* now_tm = std::localtime(&now_c);

        if (now_tm) {
            s.year = now_tm->tm_year + 1900;
            s.month = now_tm->tm_mon + 1;

        } else { // fallback
            s.year = 2026;
            s.month = 5;
        }

        return s;
    }

    void handleDateClick(std::chrono::system_clock::time_point clickedDate) {
        if (!rangeStart || (rangeStart && rangeEnd)) { // first click or resetting after second click
            rangeStart = clickedDate;
            rangeEnd = std::nullopt;
        } else { // second click
            if (clickedDate <  *rangeStart) {
                rangeStart = clickedDate;
            } else {
                rangeEnd = clickedDate;
            }
        }
    }

}; // ApplicationState

// ====

class Application {
private:

    ApplicationState m_state;
    AuthManager m_auth;

    View m_view;

public:
    Application();
    ~Application() {}

    void run();
    void shutdown();

    bool handleEvents(ftxui::Event e, ftxui::ScreenInteractive& screen);

    void debugPrintVehicles() const;

    ShortcutMap getShortcutsForContext(FocusKind focus);
}; // Application

// ==== FUNCTIONS

// ContextKind to FocusKind
static FocusKind cktofk(NavigationContextKind ck) {

    switch (ck) {
        case NONE: abort();
        case HOME: return FocusKind::HOME;
        case VEHICLE_DETAILS: return FocusKind::VEHICLE_DETAILS;
        case VEHICLE_FORM: return FocusKind::VEHICLE_FORM;
        case ADMIN_DASHBOARD: return FocusKind::ADMIN_DASHBOARD;
    }

} // cktofk


static std::unique_ptr<Vehicle> parseVehicle(const json& item) {
    // if (!item.contains("kind")) {
    //     std::cerr << "Missing kind\n";
    //     return nullptr;
    // }

    auto kind = item.at("kind").get<VehicleKind>();

    switch (kind) {
        case VehicleKind::Car: { return Car::fromJSON(item); }
        case VehicleKind::Motorcycle: { return Motorcycle::fromJSON(item); }
        case VehicleKind::Truck: { return Truck::fromJSON(item); }

        default: return nullptr;
    }
} // parseVehicle

static std::vector<std::unique_ptr<Vehicle>> loadVehicles() {
    json data = loadFile("./data.json");
    std::vector<std::unique_ptr<Vehicle>> vehicles;
    for (const auto& item : data) {
        if (auto vehicle = parseVehicle(item)) {
            vehicles.push_back(std::move(vehicle));
        }
    }

    return vehicles;
} // loadVehicles
