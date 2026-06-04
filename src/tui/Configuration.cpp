#include "Calendar.hpp"
#include "Configuration.hpp"

#include "../data/Calc.hpp"
#include "../data/Util.hpp"
#include "../util/Auth.hpp"
#include "../util/User.hpp"

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

Component constructConfigurationForm(ApplicationState& state, AuthManager& auth, Vehicle* vehicle, std::function<void(std::shared_ptr<Order>)> action, std::function<void()> onCancel) {

    auto order = std::make_shared<Order>();
    order->vehicleId = vehicle->getId();

    if (state.isSignedIn && auth.getCurrentUser()) {
        order->firstName = auth.getCurrentUser()->getFirstName();
        order->lastName = auth.getCurrentUser()->getLastName();
        order->email = auth.getCurrentUser()->getEmail();
    }

    auto isCalendarOpen = std::make_shared<bool>(false);
    auto errorMessage = std::make_shared<std::string>("");

    auto firstNameInput = Input(&(order->firstName), "Wpisz imię");
    auto lastNameInput = Input(&(order->lastName), "Wpisz nazwisko");
    auto emailInput = Input(&(order->email), "Wpisz adres e-mail");
    auto insurance = Checkbox("Pełne ubezpieczenie", &(order->wantsInsurance));

    auto createAccount = std::make_shared<bool>(false);
    auto createAccountCheckbox = Checkbox("Utwórz konto", createAccount.get());
    auto maybeCreateAccount = Maybe(createAccountCheckbox, [&state]{ return !state.isSignedIn; });

    auto dateBtn = Button("Wybierz zakres dat", [isCalendarOpen]{
        *isCalendarOpen = true;
    }, ButtonOption::Ascii());

    auto cancelBtn = Button("Anuluj", onCancel, ButtonOption::Ascii());

    auto submitBtn = Button("Potwierdź i zapłać", [&state, &auth, vehicle, action, order, errorMessage, createAccount]{
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

        if (*createAccount && !state.isSignedIn) {
            if (auth.isUserExists(order->email)) {
                *errorMessage = "Użytkownik z tym e-mailem już istnieje";
                return;
            }
            state.isOrderAccountModalOpen = true;
        } else {
            action(order);
        }
    }, ButtonOption::Ascii());

    auto mileageRadio = Radiobox(&(order->mileageOptions), &(order->mileageTier), RadioboxOption::Simple());

    auto formLayout = Container::Vertical({
        firstNameInput, // 0
        lastNameInput,  // 1
        emailInput,     // 2
        dateBtn,        // 3
        mileageRadio,   // 4
        insurance,      // 5
        maybeCreateAccount, // 6
        submitBtn,      // 7
        cancelBtn       // 8
    });

    auto styledForm = Renderer(formLayout, [&state, order, formLayout, vehicle, errorMessage]{
        bool isFocused = formLayout->Focused();

        auto errorDisplay = errorMessage->empty()
            ? text("")
            : text(*errorMessage) | color(Color::Red) | bold | hcenter;

        auto dateDisplay = (state.rangeStart && state.rangeEnd)
            ? text(format_date(*state.rangeStart) + " - " + format_date(*state.rangeEnd)) | color(Color::GrayDark)
            : text("Nie wybrano") | color(Color::GrayDark);

        auto innerContent = vbox({
            text("Dane klienta:") | bold,
            hbox({ text("Imię: "), formLayout->ChildAt(0)->Render()}),
            hbox({ text("Nazwisko: "), formLayout->ChildAt(1)->Render()}),
            hbox({ text("E-mail: "), formLayout->ChildAt(2)->Render()}),

            separator(),
            text("Okres wynajmu:") | bold,
            hbox({ formLayout->ChildAt(3)->Render(), filler(), dateDisplay }),

            separator(),

            text("Limit:") | bold,
            formLayout->ChildAt(4)->Render(),

            separator(),
            text("Opcje dodatkowe:") | bold,
            formLayout->ChildAt(5)->Render(),

            formLayout->ChildAt(6)->Render(),

            filler(),
            errorDisplay,
            separator(),
            hbox({
                formLayout->ChildAt(7)->Render() | flex,
                text(" "),
                formLayout->ChildAt(8)->Render() | flex,
            }) | hcenter,
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

    auto formWithCalendar = Modal(styledForm, modalRenderer, isCalendarOpen.get());

    // Password modal for account creation during order
    auto password = std::make_shared<std::string>();
    auto passwordError = std::make_shared<std::string>("");
    InputOption passOpt; passOpt.password = true;
    auto passwordInput = Input(password.get(), "Hasło", passOpt);

    auto confirmAccountBtn = Button("Utwórz konto i zamów", [&state, &auth, order, password, passwordError, action]{
        if (password->empty()) {
            *passwordError = "Hasło nie może być puste";
            return;
        }
        if (auth.signUp(order->email, *password, order->firstName, order->lastName)) {
            state.isOrderAccountModalOpen = false;
            auth.signIn(order->email, *password);
            state.isSignedIn = true;
            state.signedInUser = auth.getCurrentUser()->getEmail();
            action(order);
        } else {
            *passwordError = "Użytkownik już istnieje";
        }
    }, ButtonOption::Ascii());

    auto cancelAccountBtn = Button("Anuluj", [&state]{ state.isOrderAccountModalOpen = false; }, ButtonOption::Ascii());

    auto passModalContainer = Container::Vertical({
        passwordInput,
        confirmAccountBtn,
        cancelAccountBtn
    });

    auto passModalRenderer = Renderer(passModalContainer, [passModalContainer, passwordError]{
        return window(text(" Utwórz hasło dla konta "), vbox({
            hbox(text("Hasło: "), passModalContainer->ChildAt(0)->Render()),
            text(*passwordError) | color(Color::Red),
            hbox({
                passModalContainer->ChildAt(1)->Render(),
                text(" "),
                passModalContainer->ChildAt(2)->Render()
            }) | hcenter
        })) | clear_under | center;
    });

    auto formWithPass = Modal(formWithCalendar, passModalRenderer, &state.isOrderAccountModalOpen);

    // formWithPass is the only interactive root here.
    auto final = Renderer(formWithPass, [formWithPass, detailsPanel, summaryPanel]{
        return hbox({
            detailsPanel->Render(),
            formWithPass->Render(),
            summaryPanel->Render()
        }) | flex;
    });

    return final;
}
