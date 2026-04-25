#pragma once

#include "ftxui/component/app.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

// ====

typedef struct ApplicationState ApplicationState;

using namespace ftxui;

// ====

class View {
private:

    // Component m_mainContainer;

    Component m_topbar;
    Component m_content;
    Component m_rightbar;
    Component m_footer;

    Component m_breadcrumbs;

    void rebuildBreadcrumbs(ApplicationState& state);

public:

    Component m_applicationView;

    View(ApplicationState& state);
    ~View() {}

    void constructFooter(ApplicationState& state);

    Component getContent() { return m_content; }
    Component getTopbar() { return m_topbar; }

    Component getFooter() { return m_footer; }

}; // View
