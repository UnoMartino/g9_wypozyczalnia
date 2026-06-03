#include "Calendar.hpp"
#include "Configuration.hpp"

#include "../data/Calc.hpp"
#include "../data/Util.hpp"

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

        auto formatPrice = [](double price) {
            std::ostringstream stream;
            stream << std::fixed << std::setprecision(2) << price;
            return stream.str();
        };

        auto content = vbox({

            vbox({
                text("Klasa: " + std::string(tierToString(vehicle->getTier()))),
                text("Cena: " + formatPrice(calculateRentCost(vehicle->getPrice(), 1, vehicle->getTier())) + " PLN/dzień"),
            }),

            text("") | bold,
            separator(),

            vbox({std::move(details)}) | flex,

        }) | borderEmpty;

        return window(text(" Szczegóły "), content) | size(WIDTH, EQUAL, 35);;
    });
}

// Added ApplicationState& state
Component constructSummary(ApplicationState& state, Vehicle* vehicle, std::shared_ptr<Order> order) {
    return Renderer([&state, vehicle, order]{

        int days = 0;
        if (state.rangeStart && state.rangeEnd) {
            order->rentRange.start = *state.rangeStart;
            order->rentRange.end = *state.rangeEnd;

            days = order->rentRange.duration();
        }

        auto formatPrice = [](double price) {
            std::ostringstream stream;
            stream << std::fixed << std::setprecision(2) << price;
            return stream.str();
        };

        double rentPrice = (days > 0) ? calculateRentCost(vehicle->getPrice(), days, vehicle->getTier()) : 0.0;
        double insurancePrice = (order->wantsInsurance && days > 0) ? 50.00 : 0.00;
        double mileageCost = (days > 0) ? calculateMileageLimitCost(order->mileageTier, days) : 0.0;
        double totalPrice = vehicle->getPrice() + rentPrice + insurancePrice + mileageCost;

        auto content = vbox({
            text("Koszty:") | bold,
            separator(),
            hbox({ text("Zaliczka: "), filler(), text(formatPrice(vehicle->getPrice()) + " PLN") }),
            hbox({ text("Wynajem (" + std::to_string(days) + (days == 1 ? " dzień" : " dni") + "): "), filler(), text(formatPrice(rentPrice) + " PLN") }),
            hbox({ text("Przebieg: "), filler(), text(formatPrice(mileageCost) + " PLN") }),

            hbox({ text("Ubezpieczenie: "), filler(), text(formatPrice(insurancePrice) + " PLN") }),

            filler(),
            separator(),

            hbox({
                text("Razem: ") | bold,
                filler(),
                text(formatPrice(totalPrice) + " PLN") | bold | color(Color::Green)
            }),
        }) | borderEmpty | flex;

        order->price = totalPrice;

        return window(text(" Podsumowanie "), content) | size(WIDTH, EQUAL, 40);
    });
}

Component constructConfigurationForm(ApplicationState& state, Vehicle* vehicle, std::function<void(std::shared_ptr<Order>)> action) {

    auto order = std::make_shared<Order>();
    order->vehicleId = vehicle->getId();

    auto isCalendarOpen = std::make_shared<bool>(false);
    auto errorMessage = std::make_shared<std::string>("");

    auto firstNameInput = Input(&(order->firstName), "Wpisz imię");
    auto lastNameInput = Input(&(order->lastName), "Wpisz nazwisko");
    auto emailInput = Input(&(order->email), "Wpisz adres e-mail");
    auto insurance = Checkbox("Pełne ubezpieczenie", &(order->wantsInsurance));

    auto dateBtn = Button("Wybierz zakres dat", [isCalendarOpen]{
        *isCalendarOpen = true;
    }, ButtonOption::Ascii());

    auto submitBtn = Button("Potwierdź i zapłać", [&state, vehicle, action, order, errorMessage]{
        if (order->firstName.empty() || order->lastName.empty() || order->email.empty()) {
            *errorMessage = "Wypełnij wszystkie pola tekstowe";
            return;
        }

        if (order->email.find('@') == std::string::npos) {
            *errorMessage = "Nieprawidłowy email";
            return;
        }

        if (!state.rangeStart || !state.rangeEnd) {
            *errorMessage = "Wybierz termin wynajmu";
            return;
        }

        auto reservations = state.getReservations(vehicle->getId());
        for (const auto& res : reservations) {
            if (order->rentRange.start <= res.end && order->rentRange.end >= res.start) {
                *errorMessage = "Wybrany termin koliduje z inną rezerwacją";
                return;
            }
        }

        *errorMessage = ""; // Czysto, puszczamy akcję dalej
        action(order);
    }, ButtonOption::Ascii());

    auto mileageRadio = Radiobox(&(order->mileageOptions), &(order->mileageTier), RadioboxOption::Simple());

    auto formLayout = Container::Vertical({
        firstNameInput, // 1
        lastNameInput,  // 2
        emailInput,     // 3
        dateBtn,        // 4
        mileageRadio,   // 5
        insurance,      // 6
        submitBtn       // 7
    });

    auto styledForm = Renderer(formLayout, [formLayout, vehicle, errorMessage]{
        bool isFocused = formLayout->Focused();
        int i = 0;

        auto errorDisplay = errorMessage->empty()
            ? text("")
            : text(*errorMessage) | color(Color::Red) | bold | hcenter;

        auto innerContent = vbox({
            text("Dane klienta:") | bold,
            hbox({ text("Imię: "), formLayout->ChildAt(i++)->Render()}),
            hbox({ text("Nazwisko: "), formLayout->ChildAt(i++)->Render()}),
            hbox({ text("E-mail: "), formLayout->ChildAt(i++)->Render()}),

            separator(),
            text("Okres wynajmu:") | bold,
            formLayout->ChildAt(i++)->Render(),

            separator(),
            text("Limit:") | bold,
            formLayout->ChildAt(i++)->Render(),

            separator(),
            text("Opcje dodatkowe:") | bold,
            formLayout->ChildAt(i++)->Render(),

            filler(),
            errorDisplay,
            separator(),
            formLayout->ChildAt(i++)->Render() | hcenter,
        }) | borderEmpty | flex;

        auto shieldedContent = innerContent | color(Color::White);
        auto title = text(" Konfiguracja wynajmu: " + vehicle->getName() + " ") | bold | color(Color::DeepSkyBlue1);
        auto formWindow = window(title, shieldedContent) | flex | size(WIDTH, EQUAL, 60);

        if (isFocused) {
            return formWindow | color(Color::Green);
        }
        return formWindow;
    });

    auto detailsPanel  = constructDetails(vehicle);
    auto summaryPanel = constructSummary(state, vehicle, order);

    auto calendarComponent = constructCalendar(state, vehicle->getId());
    auto closeCalendarBtn = Button("Zamknij", [isCalendarOpen]{
        *isCalendarOpen = false;
    }, ButtonOption::Ascii());

    auto modalContainer = Container::Vertical({
        calendarComponent,
        closeCalendarBtn,
    });

    auto modalRenderer = Renderer(modalContainer, [modalContainer]{
        return window(text(" Wybierz daty "), vbox({
            modalContainer->ChildAt(0)->Render(),
            filler(),
            modalContainer->ChildAt(1)->Render() | hcenter,
        })) | clear_under | center;
    });

    auto formWithModal = Modal(styledForm, modalRenderer, isCalendarOpen.get());

    // formWithModal is the only interactive root here.
    auto final = Renderer(formWithModal, [formWithModal, detailsPanel, summaryPanel]{
        return hbox({
            detailsPanel->Render(),
            formWithModal->Render(),
            summaryPanel->Render()
        }) | flex;
    });

    return final;
}
