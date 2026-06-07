#include "View.hpp"
#include "Configuration.hpp"
#include "Postcard.hpp"
#include "RightPanel.hpp"
#include "OrderSummary.hpp"
#include "AdminDashboard.hpp"

#include "../Application.hpp"
#include "../util/Auth.hpp"
#include "ftxui/component/component.hpp"
#include "ftxui/component/component_options.hpp"
#include "ftxui/component/screen_interactive.hpp"

#include <ftxui/dom/elements.hpp>

#include <memory>
#include <vector>
#include <iostream>


// =====

static InputOption getWhiteInputOption(bool isPassword = false, std::function<void()> on_enter = nullptr) {
    InputOption opt;
    opt.password = isPassword;
    opt.multiline = false;
    if (on_enter) {
        opt.on_enter = on_enter;
    }
    opt.transform = [](InputState state) {
        if (state.is_placeholder) {
            state.element |= dim;
        }
        if (state.focused) {
            state.element |= bgcolor(Color::White) | color(Color::Black);
        } else if (state.hovered) {
            state.element |= underlined;
        }
        return state.element;
    };
    return opt;
}

Component makePanel(ApplicationState& state, std::string title, Component inner_content, std::function<bool()> is_focused_cb);
Component makePanel(ApplicationState& state, Component title_component, Component inner_content, std::function<bool()> is_focused_cb);

// =====

View::View(ApplicationState& state, AuthManager& auth) : m_auth(auth) {
    m_breadcrumbs = Container::Horizontal({});

    rebuildBreadcrumbs(state);

    m_userDisplay = state.isSignedIn ? state.signedInUser : "Gość";

    auto userBtn = Button(&m_userDisplay, [&state] {
            if (state.isSignedIn) state.isAccountSettingsModalOpen = true;
        },
        ButtonOption::Ascii()
    );

    auto topbarLogic = Renderer(Container::Horizontal({m_breadcrumbs, userBtn}), [this, &state, userBtn] () {
        m_userDisplay = state.isSignedIn ? state.signedInUser : "Gość";
        return hbox({
            m_breadcrumbs->Render(),
            filler(),
            userBtn->Render() | color(Color::Blue)
        }) | borderEmpty | flex;
    });

    auto btnLogin = Button("Zaloguj", [&state] { state.isLoginModalOpen = true; }, ButtonOption::Ascii());
    auto btnRegister = Button("Utwórz konto", [&state] { state.isRegisterModalOpen = true; }, ButtonOption::Ascii());
    auto btnLogout = Button("Wyloguj", [this, &state] {
        m_auth.logout();
        state.isSignedIn = false;
        state.signedInUser = "guest";
    }, ButtonOption::Ascii());

    auto btnAdminPanel = Button("Panel Administratora", [this, &state] {
        state.navigationStack.push_back({ADMIN_DASHBOARD, "Panel Administratora"});
        state.currentFocus = FocusKind::ADMIN_DASHBOARD;
        this->rebuildBreadcrumbs(state);
    }, ButtonOption::Ascii());

    auto authComponents = Container::Horizontal({
        Maybe(Container::Horizontal({btnLogin, btnRegister}), [&state]{ return !state.isSignedIn; }),
        Maybe(btnAdminPanel, [&state, this]{ return state.isSignedIn && m_auth.getCurrentUser() && m_auth.getCurrentUser()->isAdmin(); }),
        Maybe(btnLogout, [&state]{ return state.isSignedIn; }),
    });

    auto authLogic = Renderer(authComponents, [authComponents] {
        return authComponents->Render() | vcenter;
    });

    m_topbar = Container::Horizontal({
        topbarLogic,
        authLogic
    });

    Component configuration = Container::Vertical({});
    Component orderSummary = Container::Vertical({});
    auto postcard_container = Container::Vertical({});

    state.onPostcardsNeedsRebuild = [this, &state, postcard_container, configuration, orderSummary]() {
        postcard_container->DetachAllChildren();

        std::vector<Vehicle*> filtered;
        for (const auto& vehicle: state.loadedVehicles) {
            if (vehicle == nullptr) continue;
            Vehicle* ptr = vehicle.get();

            // apply filters
            if (!state.filterState.searchQuery.empty()) {
                std::string q = state.filterState.searchQuery;
                std::transform(q.begin(), q.end(), q.begin(), ::tolower);
                std::string n = ptr->getName();
                std::transform(n.begin(), n.end(), n.begin(), ::tolower);
                if (n.find(q) == std::string::npos) continue;
            }

            if (!state.filterState.showCars && ptr->getKind() == VehicleKind::Car) continue;
            if (!state.filterState.showMotorcycles && ptr->getKind() == VehicleKind::Motorcycle) continue;
            if (!state.filterState.showTrucks && ptr->getKind() == VehicleKind::Truck) continue;

            auto t = ptr->getTier();
            if (!state.filterState.showPremium && t == VehicleTier::Premium) continue;
            if (!state.filterState.showStandard && t == VehicleTier::Standard) continue;
            if (!state.filterState.showEconomy && t == VehicleTier::Economy) continue;
            if (!state.filterState.showBudget && t == VehicleTier::Budget) continue;
            if (!state.filterState.showBasic && t == VehicleTier::Basic) continue;
            if (!state.filterState.showUtility && t == VehicleTier::Utility) continue;

            try {
                if (!state.filterState.minPrice.empty()) {
                    uint32_t minP = std::stoul(state.filterState.minPrice);
                    if (ptr->getPrice() < minP) continue;
                }
            } catch (...) {}

            try {
                if (!state.filterState.maxPrice.empty()) {
                    uint32_t maxP = std::stoul(state.filterState.maxPrice);
                    if (ptr->getPrice() > maxP) continue;
                }
            } catch (...) {}

            if (state.selectionStep == 2 && state.rangeStart && state.rangeEnd) {
                bool available = true;
                for (const auto& res : state.getReservations(ptr->getId())) {
                    if (res.start <= *state.rangeEnd && res.end >= *state.rangeStart) {
                        available = false;
                        break;
                    }
                }
                if (!available) continue;
            }

            filtered.push_back(ptr);
        }

        if (state.filterState.sortOption == 1) { // Cena rosnąco
            std::sort(filtered.begin(), filtered.end(), [](Vehicle* a, Vehicle* b){ return a->getPrice() < b->getPrice(); });
        } else if (state.filterState.sortOption == 2) { // Cena malejąco
            std::sort(filtered.begin(), filtered.end(), [](Vehicle* a, Vehicle* b){ return a->getPrice() > b->getPrice(); });
        } else if (state.filterState.sortOption == 3) { // Nazwa
            std::sort(filtered.begin(), filtered.end(), [](Vehicle* a, Vehicle* b){ return a->getName() < b->getName(); });
        }

        for (auto* ptr : filtered) {
            auto p = constructPostcardComponent(ptr, [this, &state, ptr, configuration, orderSummary](){
                state.rangeStart = std::nullopt;
                state.rangeEnd = std::nullopt;
                state.selectionStep = 0;

                configuration->DetachAllChildren();
                configuration->Add(constructConfigurationForm(state, m_auth, ptr, [this, &state, orderSummary, ptr](std::shared_ptr<Order> finalOrder) {
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
            postcard_container->Add(std::move(p));
        }
    };

    state.onPostcardsNeedsRebuild();

    auto rightPanel = constructRightPanel(state);

    auto homeView = Container::Horizontal({
        postcard_container,
        rightPanel
    });

    auto adminDashboard = constructAdminDashboard(state);

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

    auto safeAdmin = Maybe(adminDashboard, [&state] {
        return state.getCurrentContext().contextId == ADMIN_DASHBOARD;
    });

    // Both safe wrappers must be inside the base tree!
    auto contentComponents = Container::Horizontal({
        safeHome,
        safeConfig,
        safeSummary,
        safeAdmin
    });

    auto contentLogic = Renderer(contentComponents, [&state, postcard_container, rightPanel, configuration, orderSummary, adminDashboard] () -> Element {
        switch (state.getCurrentContext().contextId) {
            case HOME: {
                bool anyModalOpen = state.isLoginModalOpen || state.isRegisterModalOpen || state.isAccountSettingsModalOpen || state.isOrderAccountModalOpen;
                // Focus should not be trapped here, otherwise RightPanel cannot be navigated to.
                return hbox({
                    postcard_container->Render() | vscroll_indicator | yframe,
                    rightPanel->Render(),
                }) | flex | hcenter;
            }

            case VEHICLE_DETAILS:
                return configuration->Render() | flex;

            case ORDER_SUMMARY:
                return orderSummary->Render() | flex;

            case ADMIN_DASHBOARD:
                return adminDashboard->Render() | flex;

            case VEHICLE_FORM:
                return text("TODO");

            default:
                return emptyElement();
        }
    });

    m_content = Container::Vertical({
        contentLogic | flex | hcenter,
    });

    auto topbarPanel = makePanel(state, " [0] Nawigacja", m_topbar, [&state](){
        return state.currentFocus == FocusKind::TOPBAR;
    });
    auto contentPanel = makePanel(state, Renderer([&state]{ return text(" [1] " + state.getCurrentContext().label); }), m_content, [&state]() {
        return state.currentFocus == cktofk(state.getCurrentContext().contextId);
    });

    auto combinedLayout = Renderer(Container::Vertical({ topbarPanel, contentPanel }), [topbarPanel, contentPanel] {
        return vbox({
            topbarPanel->Render(),
            contentPanel->Render() | flex,
        });
    });

    auto mainPanel =  makePanel(state, "Wypożyczalnia Januszex", combinedLayout, nullptr);

    auto loginModal = constructLoginModal(state);
    auto registerModal = constructRegisterModal(state);
    auto accountSettingsModal = constructAccountSettingsModal(state);

    m_applicationView = mainPanel;
    m_applicationView = Modal(m_applicationView, loginModal, &state.isLoginModalOpen);
    m_applicationView = Modal(m_applicationView, registerModal, &state.isRegisterModalOpen);
    m_applicationView = Modal(m_applicationView, accountSettingsModal, &state.isAccountSettingsModalOpen);

} // View::View


void View::constructFooter(ApplicationState& state) {

    m_footer = Renderer([&state] {
        Elements shortcuts = { text("[ESC] Zamknij ") | bold };

        switch (state.currentFocus) {
            case FocusKind::HOME:
                shortcuts.push_back(text("| [Enter] Wybierz | [↑/↓] Nawiguj | [/] Wyszukaj "));
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


Component View::constructLoginModal(ApplicationState& state) {
    auto email = std::make_shared<std::string>();
    auto password = std::make_shared<std::string>();
    auto error = std::make_shared<std::string>("");

    auto btnOkAction = [this, &state, email, password, error] {
        if (m_auth.signIn(*email, *password)) {
            state.isSignedIn = true;
            state.signedInUser = m_auth.getCurrentUser()->getFirstName();
            state.isLoginModalOpen = false;
            *error = "";
            *password = "";
        } else {
            *error = "Błędny e-mail lub hasło";
        }
    };

    auto inputEmail = Input(email.get(), "E-mail", getWhiteInputOption(false, btnOkAction));
    auto inputPass = Input(password.get(), "Hasło", getWhiteInputOption(true, btnOkAction));

    auto btnOk = Button("Zaloguj", btnOkAction, ButtonOption::Ascii());

    auto btnCancel = Button("Anuluj", [&state] { state.isLoginModalOpen = false; }, ButtonOption::Ascii());

    auto container = Container::Vertical({
        inputEmail,
        inputPass,
        Container::Horizontal({btnOk, btnCancel})
    });

    return Renderer(container, [container, error] {
        int i = 0;
        return window(text(" Logowanie "), vbox({
                vbox({
                    hbox(text("E-mail: "), container->ChildAt(i++)->Render()),
                    hbox(text("Hasło:  "), container->ChildAt(i++)->Render()),
                    text(*error) | color(Color::Red) | hcenter,
                    filler(),
                    container->ChildAt(2)->Render() | hcenter
                }) | borderEmpty | color(Color::White)
            })) | size(WIDTH, EQUAL, 50) | clear_under | center;
    });
} // View::constructLoginModal

Component View::constructRegisterModal(ApplicationState& state) {

    auto email = std::make_shared<std::string>();
    auto firstName = std::make_shared<std::string>();
    auto lastName = std::make_shared<std::string>();
    auto password = std::make_shared<std::string>();
    auto passwordConfirm = std::make_shared<std::string>();
    auto error = std::make_shared<std::string>("");

    auto btnOkAction = [this, &state, email, firstName, lastName, password, passwordConfirm, error] {
        if (email->empty() || password->empty() || firstName->empty() || lastName->empty()) {
            *error = "Pola nie mogą być puste";
            return;
        }

        if (*password != *passwordConfirm) {
            *error = "Hasła nie są identyczne";
            return;
        }

        if (m_auth.signUp(*email, *password, *firstName, *lastName)) {
            *error = "";
            state.isRegisterModalOpen = false;

            m_auth.signIn(*email, *password);
            state.isSignedIn = true;
            state.signedInUser = m_auth.getCurrentUser()->getEmail();

        } else {
            *error = "Taki użytkownik już istnieje";
        }
    };

    auto inputEmail = Input(email.get(), "E-mail", getWhiteInputOption(false, btnOkAction));
    auto inputFirstName = Input(firstName.get(), "Imię", getWhiteInputOption(false, btnOkAction));
    auto inputLastName = Input(lastName.get(), "Nazwisko", getWhiteInputOption(false, btnOkAction));
    auto inputPass = Input(password.get(), "Hasło", getWhiteInputOption(true, btnOkAction));
    auto inputPassConfirm = Input(passwordConfirm.get(), "Powtórz hasło", getWhiteInputOption(true, btnOkAction));


    auto btnOk = Button("Zarejestruj", btnOkAction, ButtonOption::Ascii());

    auto btnCancel = Button("Anuluj", [&state] { state.isRegisterModalOpen = false; }, ButtonOption::Ascii());

    auto container = Container::Vertical({
        inputEmail,
        inputFirstName,
        inputLastName,
        inputPass,
        inputPassConfirm,
        Container::Horizontal({btnOk, btnCancel})
    });

    return Renderer(container, [container, error] {
        int i = 0;

        return window(text(" Rejestracja "), vbox({
                vbox({
                    hbox(text("E-mail:        "), container->ChildAt(i++)->Render()),
                    hbox(text("Imię:          "), container->ChildAt(i++)->Render()),
                    hbox(text("Nazwisko:      "), container->ChildAt(i++)->Render()),
                    hbox(text("Hasło:         "), container->ChildAt(i++)->Render()),
                    hbox(text("Powtórz hasło: "), container->ChildAt(i++)->Render()),
                    text(*error) | color(Color::Red) | hcenter,
                    filler(),
                    container->ChildAt(i++)->Render() | hcenter
                }) | borderEmpty | color(Color::White),
            })) | size(WIDTH, EQUAL, 50) | clear_under | center;
    });
}


// ====

Component makePanel(ApplicationState& state, std::string title, Component inner_content, std::function<bool()> is_focused_cb) {
    return Renderer(inner_content, [title, inner_content, &state, is_focused_cb] {

        auto content = inner_content->Render() | color(Color::White);
        auto win = window(text(title) | bold | center, content);

        bool anyModalOpen = state.isLoginModalOpen || state.isRegisterModalOpen || state.isAccountSettingsModalOpen || state.isOrderAccountModalOpen;
        if (is_focused_cb && is_focused_cb() && !anyModalOpen) {
            win = win | color(Color::Green);
        }

        return win;
    });
}

Component makePanel(ApplicationState& state, Component title_component, Component inner_content, std::function<bool()> is_focused_cb) {
    auto logical_tree = Container::Vertical({
        title_component,
        inner_content
    });

    return Renderer(logical_tree, [title_component, inner_content, logical_tree, &state, is_focused_cb] {
        auto content = inner_content->Render() | color(Color::White);
        auto win = window(title_component->Render() | bold, content);

        bool anyModalOpen = state.isLoginModalOpen || state.isRegisterModalOpen || state.isAccountSettingsModalOpen || state.isOrderAccountModalOpen;
        if (is_focused_cb && is_focused_cb() && !anyModalOpen) {
            win = win | color(Color::Green);
        }

        return win;
    });
}

Component View::constructAccountSettingsModal(ApplicationState& state) {
    auto oldPassword = std::make_shared<std::string>();
    auto newPassword = std::make_shared<std::string>();
    auto confirmPassword = std::make_shared<std::string>();
    auto message = std::make_shared<std::string>("");
    auto isError = std::make_shared<bool>(false);

    auto btnOkAction = [this, &state, oldPassword, newPassword, confirmPassword, message, isError] {
        if (newPassword->empty()) {
            *message = "Nowe hasło nie może być puste";
            *isError = true;
            return;
        }

        if (*newPassword != *confirmPassword) {
            *message = "Nowe hasła nie są identyczne";
            *isError = true;
            return;
        }

        if (m_auth.changePassword(m_auth.getCurrentUser()->getEmail(), *oldPassword, *newPassword)) {
            *message = "Hasło zostało zmienione";
            *isError = false;
            *oldPassword = "";
            *newPassword = "";
            *confirmPassword = "";
        } else {
            *message = "Błędne stare hasło";
            *isError = true;
        }
    };

    auto inputOldPass = Input(oldPassword.get(), "Stare hasło", getWhiteInputOption(true, btnOkAction));
    auto inputNewPass = Input(newPassword.get(), "Nowe hasło", getWhiteInputOption(true, btnOkAction));
    auto inputConfirmPass = Input(confirmPassword.get(), "Powtórz nowe hasło", getWhiteInputOption(true, btnOkAction));

    auto btnOk = Button("Zmień hasło", btnOkAction, ButtonOption::Ascii());

    auto btnClose = Button("Zamknij", [&state, oldPassword, newPassword, confirmPassword, message] {
        state.isAccountSettingsModalOpen = false;
        *oldPassword = "";
        *newPassword = "";
        *confirmPassword = "";
        *message = "";
    }, ButtonOption::Ascii());

    auto container = Container::Vertical({
        inputOldPass,
        inputNewPass,
        inputConfirmPass,
        Container::Horizontal({btnOk, btnClose}) | hcenter,
    });

    return Renderer(container, [container, message, isError] {
        int i = 0;;

        return window(text(" Ustawienia konta ") | bold | center,
            vbox({
                vbox({
                    container->ChildAt(i++)->Render(),
                    container->ChildAt(i++)->Render(),
                    container->ChildAt(i++)->Render(),
                    text(*message) | color(*isError ? Color::Red : Color::Green) | center,

                    filler(),
                    container->ChildAt(i++)->Render(),
                }) | borderEmpty | color(Color::White),

            })
        ) | size(WIDTH, EQUAL, 40) | center | clear_under;
    });
} // View::constructAccountSettingsModal
