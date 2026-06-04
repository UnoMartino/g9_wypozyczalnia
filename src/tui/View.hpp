#pragma once

#include "ftxui/component/app.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

// ====

typedef struct ApplicationState ApplicationState;
class AuthManager;

using namespace ftxui;

// ====

class View {
private:

    // Component m_mainContainer;

    AuthManager& m_auth;

    Component m_topbar;
    Component m_content;
    Component m_rightbar;
    Component m_footer;

    Component m_breadcrumbs;
    std::string m_userDisplay;

public:

    Component m_applicationView;

    View(ApplicationState& state, AuthManager& auth);
    ~View() {}

    void constructFooter(ApplicationState& state);

    Component getContent() { return m_content; }
    Component getTopbar() { return m_topbar; }

    Component getFooter() { return m_footer; }

    void rebuildBreadcrumbs(ApplicationState& state);

    Component constructLoginModal(ApplicationState& state);
    Component constructRegisterModal(ApplicationState& state);
    Component constructAccountSettingsModal(ApplicationState& state);

}; // View
