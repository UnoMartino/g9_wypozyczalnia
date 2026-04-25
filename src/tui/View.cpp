#include "View.hpp"

#include "../Application.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/component/component_options.hpp"

#include <ftxui/dom/elements.hpp>

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

    // temporary content logic
    auto contentLogic = Renderer([&state] () -> Element {
        switch (state.getCurrentContext().contextId) {
            case HOME:
                return text("Lista pojazdów: [Toyota Corolla]");
            case VEHICLE_DETAILS:
                return text("Szczegóły: " + state.navigationStack.back().label);
            case VEHICLE_CLIENT_CONFIG:
                return text("Konfiguracja opcji...");
            default:
                return emptyElement();
        }
    });

    // temp
    auto nextButton = Button("Wejdź", [this, &state] {
        if (state.getCurrentContext().contextId == HOME) {
            state.navigationStack.push_back(NavigationNode{VEHICLE_DETAILS, "Toyota Corolla"});
            state.currentFocus = FocusKind::VEHICLE_DETAILS;
        } else if (state.getCurrentContext().contextId == VEHICLE_DETAILS) {
            state.navigationStack.push_back(NavigationNode{VEHICLE_CLIENT_CONFIG, "Konfiguruj"});
            state.currentFocus = FocusKind::VEHICLE_CLIENT_CONFIG;
        }

        this->rebuildBreadcrumbs(state);
    });

    m_topbar = Container::Horizontal({
        topbarLogic
    }) ;

    m_content = Container::Vertical({
        contentLogic,
        nextButton,
    });

    auto topbarPanel = makePanel(" [0] Nawigacja", m_topbar, [&state](){
        return state.currentFocus == FocusKind::TOPBAR;
    });
    auto contentPanel = makePanel(Renderer([&state]{ return text(" [1] " + state.getCurrentContext().label); }), m_content, [&state]() {
        // TODO! translate CONTEXT::KIND -> FOCUS::KIND
        return state.currentFocus == cktofk(state.getCurrentContext().contextId);
    });

    auto combinedLayout = Container::Vertical({ topbarPanel, contentPanel });

    auto mainPanel =  makePanel("Wypożyczalnia Januszex", combinedLayout, nullptr);

    m_applicationView = mainPanel;

} // View::View


void View::constructFooter(ApplicationState& state) {

    m_footer = Renderer([&state] {
        Elements shortcuts = { text("[ESC] Wyjście ") | bold };

        switch (state.currentFocus) {
            case FocusKind::HOME:
                shortcuts.push_back(text("| [Enter] Detale pojazdu | [↑/↓] Wybierz "));
                break;
            case FocusKind::VEHICLE_DETAILS:
                shortcuts.push_back(text("| [Enter] Konfiguruj   "));
                break;
            case FocusKind::VEHICLE_CLIENT_CONFIG:
                shortcuts.push_back(text("| [BRAK] "));
                break;
            case FocusKind::TOPBAR:
                shortcuts.push_back(text("| [←/→] Wybierz "));
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

        auto content = inner_content->Render() | color(Color::Default);
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
        auto content = inner_content->Render() | color(Color::Default);
        auto win = window(title_component->Render() | bold, content);

        if (is_focused_cb && is_focused_cb()) {
            win = win | color(Color::Green);
        }

        return win;
    });
}
