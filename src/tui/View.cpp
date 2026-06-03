#include "View.hpp"
#include "Configuration.hpp"
#include "Postcard.hpp"
#include "RightPanel.hpp"
#include "OrderSummary.hpp"

#include "../Application.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/component/component_options.hpp"
#include "ftxui/component/screen_interactive.hpp"

#include <ftxui/dom/elements.hpp>

#include <memory>
#include <vector>
#include <iostream>


// =====

Component makePanel(Component title_component, Component inner_content, std::function<bool()> is_focused_cb);
Component makePanel(std::string title, Component inner_content, std::function<bool()> is_focused_cb);

// =====

View::View(ApplicationState& state) {
    m_breadcrumbs = Container::Horizontal({});

    rebuildBreadcrumbs(state);

    auto topbarLogic = Renderer(m_breadcrumbs, [this, &state] () {

        if (state.isSignedIn) {
            return hbox({
                hbox({
                    m_breadcrumbs->Render(),
                    filler(),
                    text("Zalogowano: Gość") | color(Color::Blue)
                }) | borderEmpty | flex
            }) | flex;

        } else {
            return hbox({
                hbox({
                    m_breadcrumbs->Render(),
                    filler(),
                    text("Zalogowano: Gość") | color(Color::Blue)
                }) | borderEmpty | flex
            }) | flex;

        }
    });

    Components postcards;
    Component configuration = Container::Vertical({});
    Component orderSummary = Container::Vertical({});

    for (const auto& vehicle: state.loadedVehicles) {
        if (vehicle == nullptr) continue;

        Vehicle* ptr = vehicle.get();

        auto p = constructPostcardComponent(vehicle, [this, &state, ptr, configuration, orderSummary](){
            state.rangeStart = std::nullopt;
            state.rangeEnd = std::nullopt;
            state.selectionStep = 0;

            configuration->DetachAllChildren();
            configuration->Add(constructConfigurationForm(state, ptr, [this, &state, orderSummary, ptr](std::shared_ptr<Order> finalOrder) {
                state.orders.push_back(*finalOrder);
                saveOrders(state.orders);

                state.addReservation(ptr->getId(), finalOrder->rentRange);

                orderSummary->DetachAllChildren();
                orderSummary->Add(constructOrderSummary(state, finalOrder, [this, &state]{
                    state.navigationStack.clear();
                    state.navigationStack.push_back({HOME, "Home"});
                    state.currentFocus = FocusKind::HOME;
                    this->rebuildBreadcrumbs(state);
                }));

                state.navigationStack.push_back(NavigationNode{ORDER_SUMMARY, "Potwierdzenie"});
                state.currentFocus = FocusKind::ORDER_SUMMARY;
                this->rebuildBreadcrumbs(state);
            }, [this, &state] {
                state.navigationStack.clear();
                state.navigationStack.push_back({HOME, "Home"});
                state.currentFocus = FocusKind::HOME;
                this->rebuildBreadcrumbs(state);
            }));
            configuration->TakeFocus();

            state.navigationStack.push_back(NavigationNode{VEHICLE_DETAILS, ptr->getName()});
            state.currentFocus = FocusKind::VEHICLE_FORM;
            rebuildBreadcrumbs(state);
        });
        postcards.push_back(std::move(p));
    }
    auto postcard_container = Container::Vertical(std::move(postcards));

    auto rightPanel = constructRightPanel(state);

    auto homeView = Container::Horizontal({
        postcard_container,
        rightPanel
    });

    // This tells the engine to safely mount/unmount them from the event loop.

    auto safeHome = Maybe(homeView, [&state] {
        return state.getCurrentContext().contextId == HOME;
    });

    auto safeConfig = Maybe(configuration, [&state] {
        return state.getCurrentContext().contextId == VEHICLE_DETAILS;
    });

    auto safeSummary = Maybe(orderSummary, [&state] {
        return state.getCurrentContext().contextId == ORDER_SUMMARY;
    });

    // Both safe wrappers must be inside the base tree!
    auto contentComponents = Container::Horizontal({
        safeHome,
        safeConfig,
        safeSummary
    });

    auto contentLogic = Renderer(contentComponents, [&state, postcard_container, rightPanel, configuration, orderSummary] () -> Element {
        switch (state.getCurrentContext().contextId) {
            case HOME: {
                if (state.currentFocus == FocusKind::HOME) {
                    postcard_container->TakeFocus();
                }
                return hbox({
                    postcard_container->Render(),
                    rightPanel->Render(),
                }) | flex | hcenter;
            }

            case VEHICLE_DETAILS:
                return configuration->Render() | flex;

            case ORDER_SUMMARY:
                return orderSummary->Render() | flex;

            case VEHICLE_FORM:
                return text("TODO");

            default:
                return emptyElement();
        }
    });


    m_topbar = Container::Horizontal({
        topbarLogic
    }) ;

    m_content = Container::Vertical({
        contentLogic | flex | hcenter,

    });

    auto topbarPanel = makePanel(" [0] Nawigacja", m_topbar, [&state](){
        return state.currentFocus == FocusKind::TOPBAR;
    });
    auto contentPanel = makePanel(Renderer([&state]{ return text(" [1] " + state.getCurrentContext().label); }), m_content, [&state]() {
        return state.currentFocus == cktofk(state.getCurrentContext().contextId);
    });

    auto combinedLayout = Container::Vertical({ topbarPanel, contentPanel });

    auto mainPanel =  makePanel("Wypożyczalnia Januszex", combinedLayout, nullptr);

    m_applicationView = mainPanel;

} // View::View


void View::constructFooter(ApplicationState& state) {

    m_footer = Renderer([&state] {
        Elements shortcuts = { text("[ESC] Zamknij ") | bold };

        switch (state.currentFocus) {
            case FocusKind::HOME:
                shortcuts.push_back(text("| [Enter] Wybierz | [↑/↓] Nawiguj "));
                break;
            case FocusKind::VEHICLE_DETAILS:
                shortcuts.push_back(text("| [Backspace] Powrót "));
                break;
            case FocusKind::VEHICLE_FORM:
                shortcuts.push_back(text("| [Enter] Wybierz | [↑/↓] Nawiguj "));
                break;
            case FocusKind::ORDER_SUMMARY:
                shortcuts.push_back(text("| [Enter] Powrót do menu "));
                break;
            case FocusKind::TOPBAR:
                shortcuts.push_back(text("| [Backspace] Powrót | [←/→] Nawiguj "));
                break;
            default:
                break;
        }

        return hbox(shortcuts) | dim | center;
    });

}


void View::rebuildBreadcrumbs(ApplicationState& state) {
    m_breadcrumbs->DetachAllChildren();

    ButtonOption btnStyle;
    btnStyle.transform = [](const EntryState& s) {
        auto element = text(s.label);

        if (s.active) {
            return element | underlined | bold;
        }
        if (s.focused) {
            return element | underlined;
        }

        return element;
    };

    for (size_t i = 0; i < state.navigationStack.size(); ++i) {

        std::string labelText = state.navigationStack[i].label;

        auto btn = Button(labelText, [&state, i, this] {

            if (i < state.navigationStack.size() - 1) {
                state.navigationStack.resize(i + 1);
                this->rebuildBreadcrumbs(state);
            }
        }, btnStyle);

        m_breadcrumbs->Add(btn);

        if (i < state.navigationStack.size() - 1) {
            m_breadcrumbs->Add(Renderer([] { return text(" > ") | dim; }));
        }
    }
} // View::rebuildBreadcrumbs

// ====

Component makePanel(std::string title, Component inner_content, std::function<bool()> is_focused_cb) {
    return Renderer(inner_content, [title, inner_content, is_focused_cb] {

        auto content = inner_content->Render() | color(Color::White);
        auto win = window(text(title) | bold | center, content);

        if (is_focused_cb && is_focused_cb()) {
            win = win | color(Color::Green);
        }

        return win;
    });
}

Component makePanel(Component title_component, Component inner_content, std::function<bool()> is_focused_cb) {
    auto logical_tree = Container::Vertical({
        title_component,
        inner_content
    });

    return Renderer(logical_tree, [title_component, inner_content, logical_tree, is_focused_cb] {
        auto content = inner_content->Render() | color(Color::White);
        auto win = window(title_component->Render() | bold, content);

        if (is_focused_cb && is_focused_cb()) {
            win = win | color(Color::Green);
        }

        return win;
    });
}
