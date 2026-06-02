#include "RightPanel.hpp"
#include "View.hpp"
#include "ftxui/component/app.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/component/component_options.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/screen/screen.hpp"

#include "Calendar.hpp"

// ====

Component constructRightPanel(ApplicationState& state) {
    auto calendar = constructCalendar(state);

    return Renderer(calendar, [calendar]{
        return vbox({
            vbox({
                calendar->Render()
            }) | borderEmpty
        }) | border;
    });
} // constructRightPanel
