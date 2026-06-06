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

    static std::vector<std::string> sortEntries = {
        "Domyślne", "Cena+", "Cena-", "A-Z"
    };

    MenuOption sortOpt = MenuOption::HorizontalAnimated();
    sortOpt.on_change = [&state] {
        if (state.onPostcardsNeedsRebuild) state.onPostcardsNeedsRebuild();
    };
    auto sortMenu = Menu(&sortEntries, &state.filterState.sortOption, sortOpt);

    auto makeCheckbox = [&](const std::string& label, bool* value) {
        CheckboxOption opt = CheckboxOption::Simple();
        opt.on_change = [&state] {
            if (state.onPostcardsNeedsRebuild) state.onPostcardsNeedsRebuild();
        };
        return Checkbox(label, value, opt);
    };

    auto kindFilter = Container::Horizontal({
        makeCheckbox("Auto ", &state.filterState.showCars),
        makeCheckbox("Motocykl ", &state.filterState.showMotorcycles),
        makeCheckbox("TIR ", &state.filterState.showTrucks),
    });

    auto tierFilter1 = Container::Horizontal({
        makeCheckbox("Premium ", &state.filterState.showPremium),
        makeCheckbox("Standard ", &state.filterState.showStandard),
        makeCheckbox("Economy ", &state.filterState.showEconomy),
    });
    auto tierFilter2 = Container::Horizontal({
        makeCheckbox("Budżetowy ", &state.filterState.showBudget),
        makeCheckbox("Podstawowy ", &state.filterState.showBasic),
        makeCheckbox("Użytkowe ", &state.filterState.showUtility),
    });
    auto tierFilter = Container::Vertical({tierFilter1, tierFilter2});

    InputOption inOpt = InputOption::Default();
    inOpt.on_change = [&state] {
        if (state.onPostcardsNeedsRebuild) state.onPostcardsNeedsRebuild();
    };

    auto minPriceInput = Input(&state.filterState.minPrice, "0", inOpt);
    auto maxPriceInput = Input(&state.filterState.maxPrice, "10000", inOpt);
    auto priceFilter = Container::Horizontal({
        minPriceInput,
        maxPriceInput
    });

    auto clearDateBtn = Button("Wyczyść daty", [&state] {
        state.rangeStart = std::nullopt;
        state.rangeEnd = std::nullopt;
        state.selectionStep = 0;
        if (state.onPostcardsNeedsRebuild) state.onPostcardsNeedsRebuild();
    }, ButtonOption::Ascii());

    auto clearFiltersBtn = Button("Wyczyść filtry", [&state] {
        state.filterState = FilterState();
        if (state.onPostcardsNeedsRebuild) state.onPostcardsNeedsRebuild();
    }, ButtonOption::Ascii());

    auto searchInput = Input(&state.filterState.searchQuery, "Szukaj...", inOpt);
    state.onSearchRequested = [searchInput]() {
        searchInput->TakeFocus();
    };

    auto filtersContainer = Container::Vertical({
        calendar,
        clearDateBtn,
        sortMenu,
        kindFilter,
        tierFilter,
        priceFilter,
        clearFiltersBtn,
        searchInput
    });

    return Renderer(filtersContainer, [filtersContainer, calendar, clearDateBtn, clearFiltersBtn, searchInput, sortMenu, kindFilter, tierFilter1, tierFilter2, minPriceInput, maxPriceInput]{
        return vbox({
            vbox({
                vbox({
                    calendar->Render() | center
                }) | borderEmpty,
                clearDateBtn->Render() | center,
                separatorEmpty(),

                separator(),
                separatorEmpty(),
                hbox({ text("Sort: ") | bold, sortMenu->Render()  }) | center,
                separatorEmpty(),
                hbox({ text("Typ:  ") | bold, kindFilter->Render() }) | center,
                separatorEmpty(),
                hbox({ text("Klasa:") | bold, vbox({ tierFilter1->Render(), tierFilter2->Render() }) }) | center,
                separatorEmpty(),
                hbox({ text("Cena: ") | bold, minPriceInput->Render() | size(WIDTH, EQUAL, 5), text(" - "), maxPriceInput->Render() | size(WIDTH, EQUAL, 5) }) | center,
                separatorEmpty(),
                clearFiltersBtn->Render() | center,
                separatorEmpty(),
                separator(),
                separatorEmpty(),
                hbox({ text("Szukaj: ") | bold, searchInput->Render() | flex }) | center,
            }) | borderEmpty,

        }) | size(WIDTH, GREATER_THAN, 45) | border;
    });
} // constructRightPanel
