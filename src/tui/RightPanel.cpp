#include "RightPanel.hpp"
#include "View.hpp"
#include "ftxui/component/app.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/component/component_options.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/screen/screen.hpp"


// ====

Component constructCalendar(ApplicationState& state);

std::chrono::system_clock::time_point createTimePoint(int year, int month, int day) {
    std::tm tm = {};
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = 0;
    tm.tm_min = 0;
    tm.tm_sec = 0;
    tm.tm_isdst = -1;

    std::time_t tt = std::mktime(&tm);
    if (tt == -1) {
        return std::chrono::system_clock::time_point::min();
    }
    return std::chrono::system_clock::from_time_t(tt);
} // createTimePoint

int dateToDay(std::chrono::system_clock::time_point tp) {
    std::time_t tt = std::chrono::system_clock::to_time_t(tp);
    std::tm* timePtr = std::localtime(&tt);
    if (timePtr) {
        return timePtr->tm_mday;
    }
    return 0;
} // dateToDay


int getDaysInMonth(int year, int month) {
    static const int days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (month == 2 && ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)))
        return 29;
    return days[month - 1];
} // getDaysInMonth


int getFirstDayOffset(int year, int month) {
    std::tm tm = {};
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = 1;
    tm.tm_hour = 0;
    tm.tm_min = 0;
    tm.tm_sec = 0;
    tm.tm_isdst = -1;

    if (std::mktime(&tm) == -1) {
        return 0;
    }
    return tm.tm_wday;
} // getFirstdayOffset

// ====

Component constructCalendarGrid(ApplicationState& state);

Component constructCalendarHeader(ApplicationState& state, std::function<void()> on_month_change) {
    ButtonOption backOpt = ButtonOption::Ascii();
    backOpt.transform = [&state](const EntryState& es){
        bool canGoBack = (state.currentCalendarState.year > state.systemCalendarState.year) ||
            (state.currentCalendarState.year == state.systemCalendarState.year &&
             state.currentCalendarState.month > state.systemCalendarState.month);

        if (!canGoBack) return text("<") | color(Color::GrayDark);
        else return text("<") | (es.focused ? inverted : nothing);
    };

    return Container::Horizontal({
        Button("<", [&state, on_month_change](){
            bool canGoBack = (state.currentCalendarState.year > state.systemCalendarState.year) ||
                (state.currentCalendarState.year == state.systemCalendarState.year &&
                 state.currentCalendarState.month > state.systemCalendarState.month);
            if (!canGoBack) return; // do nothing

            state.screen.Post([&state, on_month_change]{
                if (--state.currentCalendarState.month < 1) {
                    state.currentCalendarState.month = 12;
                    state.currentCalendarState.year--;
                }
                // REVIEW
                on_month_change();
            });
        }, backOpt),

        Renderer([&state]{
            std::vector<std::string> months = {"Styczeń", "Luty", "Marzec", "Kwiecień", "Maj", "Czerwiec",
            "Lipiec", "Sierpień", "Wrzesień", "Październik", "Listopad", "Grudzień"};

            return text(months[state.currentCalendarState.month - 1] + " " + std::to_string(state.currentCalendarState.year))
                    | bold
                    | hcenter
                    | flex;
        }),

        Button(">", [&state, on_month_change] {
            state.screen.Post([&state, on_month_change] {
                if (++state.currentCalendarState.month > 12) {
                    state.currentCalendarState.month = 1;
                    state.currentCalendarState.year++;
                }

                // REVIEW
                on_month_change();
            });
        }, ButtonOption::Ascii()),
    });
}

// REVIEW
Component constructCalendarGrid(ApplicationState& state) {
    auto grid = Container::Vertical({});
    auto current = state.currentCalendarState;

    grid->Add(Renderer([]{
        return hbox({
            text("Pn") | hcenter | size(WIDTH, EQUAL, 4),
            text("Wt") | hcenter | size(WIDTH, EQUAL, 4),
            text("Śr") | hcenter | size(WIDTH, EQUAL, 4),
            text("Cz") | hcenter | size(WIDTH, EQUAL, 4),
            text("Pt") | hcenter | size(WIDTH, EQUAL, 4),
            text("So") | hcenter | size(WIDTH, EQUAL, 4),
            text("Nd") | hcenter | size(WIDTH, EQUAL, 4),
        }) | color(Color::GrayDark);
    }));

    auto currentRow = Container::Horizontal({});

    int offset = getFirstDayOffset(current.year, current.month);
    int padding = (offset == 0) ? 6 : offset - 1;

    for (int p = 0; p < padding; ++p) {
        currentRow->Add(Renderer([]{ return text("    ") | size(WIDTH, EQUAL, 4); }));
    }

    int dayInMonth = getDaysInMonth(current.year, current.month);

    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm* now_tm = std::localtime(&now_c);
    auto today = createTimePoint(now_tm->tm_year + 1900, now_tm->tm_mon + 1, now_tm->tm_mday);

    for (int day = 1; day <= dayInMonth; ++day) {
        auto date = createTimePoint(current.year, current.month, day);
        bool isPast = date < today;

        ButtonOption opt;
        opt.transform = [&state, date, isPast] (const EntryState& es) {
            auto element = text(es.label) | hcenter | size(WIDTH, EQUAL, 4);

            if (isPast) return element | color(Color::GrayDark);

            if (state.rangeStart && date == *state.rangeStart) element |= bgcolor(Color::Blue);
            else if (state.rangeEnd && date == *state.rangeEnd) element |= bgcolor(Color::Blue);
            else if (state.rangeStart && state.rangeEnd && date > *state.rangeStart && date < *state.rangeEnd) element |= bgcolor(Color::DeepSkyBlue1);

            if (es.focused) element | inverted;
            return element;
        };

        currentRow->Add(Button(std::to_string(day), [&state, date, isPast] {
            if (isPast) return;
            state.handleDateClick(date);
        }, opt));

        if ((day + padding) % 7 == 0 || day == dayInMonth) {
            grid->Add(currentRow);
            currentRow = Container::Horizontal({});
        }
    }

    return grid;
}

Component constructCalendar(ApplicationState& state) {
    auto grid_container = Container::Vertical({});

    grid_container->Add(constructCalendarGrid(state));

    auto rebuild_grid = [&state, grid_container]() {
        grid_container->DetachAllChildren();
        grid_container->Add(constructCalendarGrid(state));
    };

    auto header = constructCalendarHeader(state, rebuild_grid);

    return Container::Vertical({
        header,
        grid_container
    });
}


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
