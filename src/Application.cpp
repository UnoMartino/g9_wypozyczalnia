#include "Application.hpp"

#include "data/Loader.hpp"

#include "data/vehicle/Vehicle.hpp"
#include "data/vehicle/Car.hpp"
#include "data/vehicle/Motorcycle.hpp"
#include "data/vehicle/Truck.hpp"
#include "ftxui/component/component_base.hpp"
#include "tui/View.hpp"

#include <iostream>
#include <string>

// ====


// ====

// load the application state and data
void Application::run() {

    json data = loadFile("./data.json");
    for (const auto& item : data) {
        if (auto vehicle = parseVehicle(item)) {
            m_state.loadedVehicles.push_back(std::move(vehicle));
        }
    }

    auto screen = ftxui::ScreenInteractive::Fullscreen();

    // catch main_layout 'ESC' key -> application safely exits
    m_view.constructFooter(m_state);

    auto application_layout = Container::Vertical({
        m_view.m_applicationView | flex,
        m_view.getFooter()
    });

    application_layout = ftxui::CatchEvent(application_layout, [this, &screen](ftxui::Event e) {
        return handleEvents(e, screen);
    });

    screen.Loop(application_layout);

} // Application::run


bool Application::handleEvents(ftxui::Event e, ftxui::ScreenInteractive& screen) {

    if (e == ftxui::Event::Escape) {
        screen.ExitLoopClosure()();
        // save the application data to files
        shutdown();
        return true;
    }

    if (e == ftxui::Event::Character('0')) {
        m_state.currentFocus = FocusKind::TOPBAR;
        m_view.getTopbar()->TakeFocus();
        return true;
    }

    /*
        temporary
        focus here should be translated based on current Content of the Content Context
        TODO! translate CONTEXT::KIND -> FOCUS::KIND
        */
    if (e == ftxui::Event::Character('1')) {
        m_state.currentFocus = cktofk(m_state.getCurrentContext().contextId);
        m_view.getContent()->TakeFocus();
        return true;
    }


    FocusKind currentContext = m_state.currentFocus;
    ShortcutMap shortcuts = getShortcutsForContext(currentContext);

    for (const auto& [triggerEvent, action] : shortcuts) {
        if (e == triggerEvent) {
            if (action) action();
            return true;
        }
    }

    return false;
} // Application::handleEvents


std::unique_ptr<Vehicle> Application::parseVehicle(const json& item) {
    if (!item.contains("kind")) {
        std::cerr << "Missing kind\n";
        return nullptr;
    }

    auto kind = item.at("kind").get<VehicleKind>();

    switch (kind) {
        case VehicleKind::Car: { return Car::fromJSON(item); }
        case VehicleKind::Motorcycle: { return Motorcycle::fromJSON(item); }
        case VehicleKind::Truck: { return Truck::fromJSON(item); }

        default: return nullptr;
    }
} // Application::parseVehicle

// saves the application state and data
// prepares to exit the application
void Application::shutdown() {
    m_state.isRunning = false;
} // Application::shutdown


ShortcutMap Application::getShortcutsForContext(FocusKind focus) {
    switch (focus) {

        case FocusKind::TOPBAR: return {
            {Event::Character('z'), []{
                // show popup with sign in screen
            }},

            {Event::Character('Z'), []{
                // show popup with sign up screen
            }},

            // optional, shows only when user is signed in
            // {Event::Character('w'), []{ /* logout */ }},

            {Event::Backspace, []{
                // if possible, switch context to previous
            }}
        };

        case FocusKind::HOME: return {

        };

    }

    return {};
} // Application::getShortcutsForContext


// ==== DEBUG

void Application::debugPrintVehicles() const {
    std::cout << "\n [DEBUG] Loaded vehicles:\n";

    for (const auto& vehicle : m_state.loadedVehicles) {
        if (vehicle) {
            vehicle->printInfo();
            std::cout << "----------------------------------------\n";
        }
    }
} // Application::debugPrintVehicles
