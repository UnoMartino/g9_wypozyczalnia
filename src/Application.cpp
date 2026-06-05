#include "Application.hpp"


// #include "data/Vehicle.hpp"
#include "data/Order.hpp"
#include "data/vehicle/Vehicle.hpp"

#include "ftxui/component/component_base.hpp"
#include "tui/View.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <fstream>

// ====

// ====

Application::Application() : m_view(m_state, m_auth) {
    m_state.currentCalendarState = m_state.systemCalendarState;

    // populate reservations from loaded orders
    for (const auto& order : m_state.orders) {
        m_state.addReservation(order.vehicleId, order.rentRange);
    }
}

// load the application state and data
void Application::run() {

    // catch main_layout 'ESC' key -> application safely exits
    m_view.constructFooter(m_state);

    auto application_layout = Container::Vertical({
        m_view.m_applicationView | ftxui::flex,
        m_view.getFooter()
    });

    application_layout = ftxui::CatchEvent(application_layout, [this](ftxui::Event e) {
        return handleEvents(e, m_state.screen);
    });

    m_state.screen.Loop(application_layout);

} // Application::run


bool Application::handleEvents(ftxui::Event e, ftxui::ScreenInteractive& screen) {

    if (e == ftxui::Event::Escape) {
        screen.ExitLoopClosure()();
        shutdown();
        return true;
    }

    // if an input is focused, the application should not process global shortcuts
    bool inputFocused = false;
    if (m_view.m_applicationView) {
        inputFocused = m_view.m_applicationView->Focused();
    }

    if (e == ftxui::Event::Character('0')) {
        if (inputFocused) return false; // Let the input handle it
        m_state.currentFocus = FocusKind::TOPBAR;
        m_view.getTopbar()->TakeFocus();
        return true;
    }


    if (e == ftxui::Event::Character('1')) {
        if (inputFocused) return false; // Let the input handle it
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



// saves the application state and data
// prepares to exit the application
void Application::shutdown() {

    //
    saveOrders(m_state.orders);

    m_state.isRunning = false;
} // Application::shutdown


ShortcutMap Application::getShortcutsForContext(FocusKind focus) {

    // backspace is always the same
    ShortcutMap shortcuts = {

    };

    switch (focus) {
        case FocusKind::TOPBAR: {
            shortcuts.insert({
                {Event::Character('z'), []{
                    // show popup with sign in screen
                }},

                {Event::Character('Z'), []{
                    // show popup with sign up screen
                }},

                {Event::Backspace, [this] {
                    if (m_state.navigationStack.size() > 1) {
                        m_state.navigationStack.pop_back();
                        m_state.currentFocus = cktofk(m_state.navigationStack.back().contextId);

                        m_view.rebuildBreadcrumbs(m_state);
                    }
                }},
            });
        } break;

        case FocusKind::HOME: {
            shortcuts.insert({

            });
        } break;

        case FocusKind::VEHICLE_DETAILS: {
            shortcuts.insert({

            });
        } break;

    }

    return shortcuts;
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
