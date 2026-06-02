#include "Concfiguration.hpp"

#include "ftxui/component/app.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/component/component_options.hpp"
#include "ftxui/dom/elements.hpp"

#include <memory>
#include <functional>


// ====

Component constructDetails(Vehicle* vehicle) {
    return Renderer([vehicle]{
        Elements details;

        auto detail_map = vehicle->getDetails();
        for (const auto& [key, value] : detail_map) {
            details.push_back(hbox({ text(key + ": " + value), filler() }));
        }

        auto content = vbox({

            hbox({
                text("Klasa: " + std::string(tierToString(vehicle->getTier()))),
                filler(),
                text("Cena: " + std::to_string(vehicle->getPrice()) + " PLN"),
            }),

            text("") | bold,
            separator(),

            vbox({std::move(details)}) | flex,

        }) | borderEmpty;

        return window(text(" Szczegóły "), content) | size(WIDTH, EQUAL, 35);;
    });
}

Component constructSummary(Vehicle* vehicle, std::shared_ptr<Order> order) {
    return Renderer([vehicle, order]{

        int totalPrice = vehicle->getPrice();
        int insurancePrice = order->wantsInsurance ? 50 : 0;
        totalPrice += insurancePrice;

        auto content = vbox({
            text("Koszty:") | bold,
            separator(),
            hbox({ text("Wynajem: "), filler(), text(std::to_string(vehicle->getPrice()) + " PLN") }),
            hbox({ text("Ubezpieczenie: "), filler(), text(std::to_string(insurancePrice) + " PLN") }),

            filler(),
            separator(),

            hbox({
                text("Razem: ") | bold,
                filler(),
                text(std::to_string(totalPrice) + " PLN") | bold | color(Color::Green)
            }),
        }) | borderEmpty | flex;

        order->price = totalPrice;

        return window(text(" Podsumowanie "), content) | size(WIDTH, EQUAL, 40);
    });
}

Component constructConfigurationForm(Vehicle* vehicle, std::function<void(std::shared_ptr<Order>)> action) {

    auto order = std::make_shared<Order>();
    auto isCalendarOpen = std::make_shared<bool>();

    auto firstNameInput = Input(&(order->firstName), "Wpisz imię");
    auto lastNameInput = Input(&(order->lastName), "Wpisz nazwisko");
    auto emailInput = Input(&(order->email), "Wpisz adres e-mail");

    auto dateBtn = Button("Wybierz zakres dat", [isCalendarOpen]{
        *isCalendarOpen = true;

    }, ButtonOption::Ascii());

    // button that shows the popup with calendar for range selection
    // calendar should disable days that are already taken

    auto insurance = Checkbox("Pełne ubezpieczenie", &(order->wantsInsurance));

    auto submitBtn = Button("Potwierdź i zapłać", [action, order]{
        action(order);
    }, ButtonOption::Ascii());

    auto formLayout = Container::Vertical({
        firstNameInput,
        lastNameInput,
        emailInput,

        insurance,
        dateBtn,

        submitBtn
    });

    auto detailsPanel  = constructDetails(vehicle);
    auto summaryPanel = constructSummary(vehicle, order);


    auto combinedLayout = Container::Horizontal({
        detailsPanel,
        formLayout,
        summaryPanel
    });

    // REVIEW
    // auto final = Renderer(formLayout, [detailsPanel, formLayout, summaryPanel, vehicle]{
    auto final = Renderer(combinedLayout, [detailsPanel, formLayout, summaryPanel, vehicle]{
        bool isFocused = formLayout->Focused();

        int i = 0;
        auto innerContent = vbox({
            text("Dane klienta:") | bold,
            hbox({ text("Imię: "), formLayout->ChildAt(i++)->Render()}),
            hbox({ text("Nazwisko: "), formLayout->ChildAt(i++)->Render()}),
            hbox({ text("E-mail: "), formLayout->ChildAt(i++)->Render()}),

            separator(),
            text("Opcje dodatkowe:") | bold,
            formLayout->ChildAt(i++)->Render(),

            filler(),
            separator(),
            formLayout->ChildAt(i++)->Render() | hcenter,
        }) | borderEmpty | flex;

        auto shieldedContent = innerContent | color(Color::White);
        auto title = text(" Konfiguracja wynajmu: " + vehicle->getName() + " ") | bold | color(Color::DeepSkyBlue1);
        auto formWindow = window(title, shieldedContent) | flex;

        if (isFocused) {
            formWindow = formWindow | color(Color::Green);
        }

        return hbox({
            detailsPanel->Render(),
            formWindow,
            summaryPanel->Render()
        }) | flex;
    });

    return final;
} // constructConfigurationForm
