#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "data/vehicle/Vehicle.hpp"

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

// ====

enum class FocusKind {
    TOPBAR,
    HOME,
    VEHICLE_DETAILS,
    VEHICLE_CLIENT_CONFIG,
    ADMIN_DASHBOARD,
    RIGHTBAR,
};

enum NavigationContextKind {
    NONE,
    HOME,
    VEHICLE_DETAILS,
    VEHICLE_CLIENT_CONFIG,
    ADMIN_DASHBOARD,
};

struct NavigationNode {
    NavigationContextKind contextId;
    std::string label;
};

// stores application state data
struct ApplicationState {
    std::vector<std::unique_ptr<Vehicle>> loadedVehicles;       // store vehicle list from 'data.json'

    std::vector<NavigationNode> navigationStack = {NavigationNode{HOME, "Home"}};
    FocusKind currentFocus = FocusKind::HOME;

    bool isRunning = true;                                      // is application running

    NavigationNode getCurrentContext() { return navigationStack.back(); }

    bool isSignedIn = false;
    std::string signedInUser = "guest";

}; // ApplicationState

// ====

class Application {
private:
    ApplicationState m_state;
    AuthManager m_auth;

    View m_view;

    std::unique_ptr<Vehicle> parseVehicle(const json& j);

public:
    Application() : m_view(m_state) {}
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
        case VEHICLE_CLIENT_CONFIG: return FocusKind::VEHICLE_CLIENT_CONFIG;
        case ADMIN_DASHBOARD: return FocusKind::ADMIN_DASHBOARD;
    }

}
