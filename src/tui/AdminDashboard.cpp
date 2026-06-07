#include "AdminDashboard.hpp"
#include "ftxui/component/component_options.hpp"
#include "../data/vehicle/Car.hpp"
#include "../data/vehicle/Motorcycle.hpp"
#include "../data/vehicle/Truck.hpp"
#include "../data/Order.hpp"
#include "ftxui/dom/elements.hpp"
#include <string>
#include <memory>
#include <vector>
#include "ftxui/component/event.hpp"
#include "ftxui/component/mouse.hpp"
using namespace ftxui;

namespace {
    struct FormState {
        std::shared_ptr<std::string> modelName = std::make_shared<std::string>();
        std::shared_ptr<std::string> licensePlate = std::make_shared<std::string>();
        std::shared_ptr<std::string> price = std::make_shared<std::string>();
        std::shared_ptr<std::string> mileage = std::make_shared<std::string>();
        std::shared_ptr<std::string> capacity = std::make_shared<std::string>();

        std::shared_ptr<int> kindIndex = std::make_shared<int>(0);
        std::shared_ptr<std::vector<std::string>> kindOptions = std::make_shared<std::vector<std::string>>(std::vector<std::string>{ "Samochód", "Motocykl", "Ciężarówka" });
        std::shared_ptr<int> tierIndex = std::make_shared<int>(0);
        std::shared_ptr<std::vector<std::string>> tierOptions = std::make_shared<std::vector<std::string>>(std::vector<std::string>{ "Premium", "Standard", "Economy", "Budget", "Basic", "Utility" });
        std::shared_ptr<std::string> resultMsg = std::make_shared<std::string>();

        std::shared_ptr<std::string> carTrunk = std::make_shared<std::string>();
        std::shared_ptr<int> carBodyStyleIndex = std::make_shared<int>(0);
        std::shared_ptr<std::vector<std::string>> carBodyStyleOptions = std::make_shared<std::vector<std::string>>(std::vector<std::string>{ "Sedan", "SUV", "Kombi", "Hatchback", "Inne" });
        std::shared_ptr<bool> carIsElectric = std::make_shared<bool>(false);

        std::shared_ptr<bool> motoHasSidecar = std::make_shared<bool>(false);
        std::shared_ptr<bool> motoRequiresFullLicense = std::make_shared<bool>(false);
        std::shared_ptr<bool> motoHasLuggage = std::make_shared<bool>(false);

        std::shared_ptr<std::string> truckMaxPayload = std::make_shared<std::string>();
        std::shared_ptr<int> truckTrailerIndex = std::make_shared<int>(0);
        std::shared_ptr<std::vector<std::string>> truckTrailerOptions = std::make_shared<std::vector<std::string>>(std::vector<std::string>{ "Chłodnia", "Cysterna", "TIR", "Brak" });
        std::shared_ptr<bool> truckHasSleeperCab = std::make_shared<bool>(false);

        void clear() {
            *modelName = ""; *licensePlate = ""; *price = ""; *mileage = ""; *capacity = "";
            *carTrunk = ""; *carIsElectric = false;
            *motoHasSidecar = false; *motoRequiresFullLicense = false; *motoHasLuggage = false;
            *truckMaxPayload = ""; *truckHasSleeperCab = false;
        }

        void syncFromJSON(const nlohmann::json& j) {
            *modelName = (j.contains("modelName") && j["modelName"].is_string()) ? j["modelName"].get<std::string>() : "";
            *licensePlate = (j.contains("licensePlate") && j["licensePlate"].is_string()) ? j["licensePlate"].get<std::string>() : "";
            *price = std::to_string(j.value("price", 0));
            *mileage = std::to_string(j.value("mileage", 0));
            *capacity = std::to_string(j.value("passengerCapacity", 0));

            int tierVal = j.value("tier", 1) - 1;
            if (tierVal < 0 || tierVal >= (int)tierOptions->size()) tierVal = 0;
            *tierIndex = tierVal;

            int kindVal = j.value("kind", 0);
            if (kindVal < 0 || kindVal >= (int)kindOptions->size()) kindVal = 0;
            *kindIndex = kindVal;

            if (*kindIndex == 0) { // Car
                *carTrunk = std::to_string(j.value("trunkCapacityLiters", 0));
                *carIsElectric = j.value("isElectric", false);
                if (j.contains("bodyStyle") && !j["bodyStyle"].is_null()) {
                    int bs = j["bodyStyle"].get<int>();
                    if (bs < 0 || bs >= (int)carBodyStyleOptions->size()) bs = 4;
                    *carBodyStyleIndex = bs;
                } else {
                    *carBodyStyleIndex = 4;
                }
            } else if (*kindIndex == 1) { // Moto
                *motoHasSidecar = j.value("hasSidecar", false);
                *motoRequiresFullLicense = j.value("requiresFullLicense", false);
                *motoHasLuggage = j.value("hasLuggagePanniers", false);
            } else if (*kindIndex == 2) { // Truck
                *truckMaxPayload = std::to_string(j.value("maxPayloadKg", 0));
                *truckHasSleeperCab = j.value("hasSleeperCab", false);
                if (j.contains("trailerType") && !j["trailerType"].is_null()) {
                    int tt = j["trailerType"].get<int>();
                    if (tt < 0 || tt >= (int)truckTrailerOptions->size()) tt = 3;
                    *truckTrailerIndex = tt;
                } else {
                    *truckTrailerIndex = 3;
                }
            }
        }
    };

    Component buildForm(ApplicationState& state, std::shared_ptr<FormState> fs, bool isEditMode, std::shared_ptr<int> selectedVehicleIndex = nullptr) {
        auto numericFilter = [](std::shared_ptr<std::string> str) {
            std::string filtered;
            for (char c : *str) {
                if (std::isdigit(static_cast<unsigned char>(c))) {
                    filtered += c;
                }
            }
            *str = filtered;
        };

        InputOption opt;
        opt.multiline = false;

        InputOption numOpt = opt;

        auto inputModel = Input(fs->modelName.get(), "Nazwa modelu", opt);

        auto optPrice = numOpt;
        optPrice.on_change = [fs, numericFilter] { numericFilter(fs->price); };
        auto inputPrice = Input(fs->price.get(), "Cena (PLN/dzień)", optPrice);

        auto optMileage = numOpt;
        optMileage.on_change = [fs, numericFilter] { numericFilter(fs->mileage); };
        auto inputMileage = Input(fs->mileage.get(), "Przebieg", optMileage);

        auto optCapacity = numOpt;
        optCapacity.on_change = [fs, numericFilter] { numericFilter(fs->capacity); };
        auto inputCapacity = Input(fs->capacity.get(), "Pojemność/Liczba miejsc", optCapacity);

        auto inputLicensePlate = Input(fs->licensePlate.get(), "Numer rejestracyjny", opt);

        auto kindRadio = Radiobox(fs->kindOptions.get(), fs->kindIndex.get());
        auto tierRadio = Radiobox(fs->tierOptions.get(), fs->tierIndex.get());

        auto optTrunk = numOpt;
        optTrunk.on_change = [fs, numericFilter] { numericFilter(fs->carTrunk); };
        auto inputCarTrunk = Input(fs->carTrunk.get(), "Pojemność bagażnika (L)", optTrunk);

        auto radioCarBody = Radiobox(fs->carBodyStyleOptions.get(), fs->carBodyStyleIndex.get());
        auto checkCarElectric = Checkbox("Pojazd elektryczny", fs->carIsElectric.get());

        auto checkMotoSidecar = Checkbox("Wózek boczny", fs->motoHasSidecar.get());
        auto checkMotoLicense = Checkbox("Wymaga kat. A", fs->motoRequiresFullLicense.get());
        auto checkMotoLuggage = Checkbox("Kufry", fs->motoHasLuggage.get());

        auto optPayload = numOpt;
        optPayload.on_change = [fs, numericFilter] { numericFilter(fs->truckMaxPayload); };
        auto inputTruckPayload = Input(fs->truckMaxPayload.get(), "Max ładowoność (kg)", optPayload);

        auto radioTruckTrailer = Radiobox(fs->truckTrailerOptions.get(), fs->truckTrailerIndex.get());
        auto checkTruckSleeper = Checkbox("Kabina sypialna", fs->truckHasSleeperCab.get());

        auto btnSubmit = Button(isEditMode ? "Aktualizuj" : "Zapisz", [&state, fs, isEditMode, selectedVehicleIndex] {
            if (fs->modelName->empty() || fs->price->empty()) {
                *fs->resultMsg = "Błąd: Nazwa i cena są wymagane.";
                return;
            }

            if (isEditMode && (state.loadedVehicles.empty() || !selectedVehicleIndex || *selectedVehicleIndex >= static_cast<int>(state.loadedVehicles.size()) || !state.loadedVehicles[*selectedVehicleIndex])) {
                *fs->resultMsg = "Błąd: Nie wybrano pojazdu.";
                return;
            }

            // Duplicate license plate check
            if (!fs->licensePlate->empty()) {
                for (const auto& v : state.loadedVehicles) {
                    if (v) {
                        auto lp = v->getLicensePlate();
                        if (lp && *lp == *fs->licensePlate) {
                            if (!isEditMode || (selectedVehicleIndex && v->getId() != state.loadedVehicles[*selectedVehicleIndex]->getId())) {
                                *fs->resultMsg = "Błąd: Pojazd z tą rejestracją już istnieje!";
                                return;
                            }
                        }
                    }
                }
            }

            VehicleData vd;
            if (isEditMode) {
                auto oldVehicle = state.loadedVehicles[*selectedVehicleIndex].get();
                if (!oldVehicle) {
                    *fs->resultMsg = "Błąd: Dane pojazdu są nieprawidłowe.";
                    return;
                }
                vd.id = oldVehicle->getId();

                auto oldJson = oldVehicle->toJSON();
                vd.needsService = oldJson.value("needsService", false);
            } else {
                vd.id = 0;
                for (const auto& v : state.loadedVehicles) {
                    if (v && v->getId() >= vd.id) {
                        vd.id = v->getId() + 1;
                    }
                }
            }

            vd.modelName = *fs->modelName;
            if (!fs->licensePlate->empty()) {
                vd.licensePlate = *fs->licensePlate;
            } else {
                vd.licensePlate = std::nullopt;
            }
            try {
                vd.price = std::stoi(*fs->price);
                vd.mileage = fs->mileage->empty() ? 0 : std::stoi(*fs->mileage);
                vd.passengerCapacity = fs->capacity->empty() ? 0 : std::stoi(*fs->capacity);
            } catch(...) {
                *fs->resultMsg = "Błąd: Nieprawidłowy format liczby (Cena/Przebieg/Miejsca).";
                return;
            }
            vd.tier = static_cast<VehicleTier>(*fs->tierIndex + 1);

            std::unique_ptr<Vehicle> newVehicle;

            if (*fs->kindIndex == 0) {
                vd.kind = VehicleKind::Car;
                CarData cd;
                cd.base = vd;
                try {
                    cd.trunkCapacityLiters = fs->carTrunk->empty() ? 0 : std::stoi(*fs->carTrunk);
                } catch(...) {
                    *fs->resultMsg = "Błąd: Nieprawidłowa pojemność bagażnika.";
                    return;
                }
                BodyStyle bs[] = { Sedan, SUV, StateWagon, Hatchback, None };
                cd.bodyStyle = bs[*fs->carBodyStyleIndex];
                cd.isElectric = *fs->carIsElectric;
                newVehicle = std::make_unique<Car>(cd);
            } else if (*fs->kindIndex == 1) {
                vd.kind = VehicleKind::Motorcycle;
                MotorcycleData md;
                md.base = vd;
                md.hasSidecar = *fs->motoHasSidecar;
                md.requiresFullLicense = *fs->motoRequiresFullLicense;
                md.hasLuggagePanniers = *fs->motoHasLuggage;
                newVehicle = std::make_unique<Motorcycle>(md);
            } else if (*fs->kindIndex == 2) {
                vd.kind = VehicleKind::Truck;
                TruckData td;
                td.base = vd;
                try {
                    td.maxPayloadKg = fs->truckMaxPayload->empty() ? 0 : std::stoi(*fs->truckMaxPayload);
                } catch(...) {
                    *fs->resultMsg = "Błąd: Nieprawidłowa ładowność.";
                    return;
                }
                TrailerType tt[] = { Refrigerated, Tank, Dry, Unknown };
                td.trailerType = tt[*fs->truckTrailerIndex];
                td.hasSleeperCab = *fs->truckHasSleeperCab;
                newVehicle = std::make_unique<Truck>(td);
            }

            if (isEditMode) {
                state.loadedVehicles[*selectedVehicleIndex] = std::move(newVehicle);
                *fs->resultMsg = "Sukces: Zaktualizowano!";
            } else {
                state.loadedVehicles.push_back(std::move(newVehicle));
                *fs->resultMsg = "Sukces: Pojazd dodany!";
                fs->clear();
            }
            saveVehicles(state.loadedVehicles);

        }, ButtonOption::Ascii());

        auto carFields = Container::Vertical({
            inputCarTrunk, radioCarBody, checkCarElectric
        });
        auto carLogic = Renderer(carFields, [carFields] {
            return vbox({
                separatorEmpty(),
                text("Szczegóły samochodu:") | bold,
                hbox({ text("Bagażnik (L): ") | size(WIDTH, EQUAL, 25), carFields->ChildAt(0)->Render() }),
                hbox({ text("Nadwozie: ") | size(WIDTH, EQUAL, 25), carFields->ChildAt(1)->Render() }),
                carFields->ChildAt(2)->Render()
            });
        });
        auto carMaybe = Maybe(carLogic, [fs] { return *fs->kindIndex == 0; });

        auto motoFields = Container::Vertical({
            checkMotoSidecar, checkMotoLicense, checkMotoLuggage
        });
        auto motoLogic = Renderer(motoFields, [motoFields] {
            return vbox({
                separatorEmpty(),
                text("Szczegóły motocykla:") | bold,
                motoFields->ChildAt(0)->Render(),
                motoFields->ChildAt(1)->Render(),
                motoFields->ChildAt(2)->Render()
            });
        });
        auto motoMaybe = Maybe(motoLogic, [fs] { return *fs->kindIndex == 1; });

        auto truckFields = Container::Vertical({
            inputTruckPayload, radioTruckTrailer, checkTruckSleeper
        });
        auto truckLogic = Renderer(truckFields, [truckFields] {
            return vbox({
                separatorEmpty(),
                text("Szczegóły ciężarówki:") | bold,
                hbox({ text("Ładowność (kg): ") | size(WIDTH, EQUAL, 25), truckFields->ChildAt(0)->Render() }),
                hbox({ text("Typ naczepy: ") | size(WIDTH, EQUAL, 25), truckFields->ChildAt(1)->Render() }),
                truckFields->ChildAt(2)->Render()
            });
        });
        auto truckMaybe = Maybe(truckLogic, [fs] { return *fs->kindIndex == 2; });

        auto formContainer = Container::Vertical({
            inputModel, inputLicensePlate, inputPrice, inputMileage, inputCapacity,
            isEditMode ? Renderer([]{ return text(""); }) : kindRadio,
            tierRadio,
            carMaybe, motoMaybe, truckMaybe,
            btnSubmit
        });

        auto formContainerWithMouseScroll = CatchEvent(formContainer, [formContainer](Event e) {
            if (e.is_mouse()) {
                if (e.mouse().button == Mouse::WheelDown) {
                    formContainer->OnEvent(Event::Tab);
                    return true;
                }
                if (e.mouse().button == Mouse::WheelUp) {
                    formContainer->OnEvent(Event::TabReverse);
                    return true;
                }
            }
            return false;
        });

        return Renderer(formContainerWithMouseScroll, [formContainer, fs, isEditMode] {
            auto kindText = (isEditMode && *fs->kindIndex >= 0 && *fs->kindIndex < (int)fs->kindOptions->size())
                ? fs->kindOptions->at(*fs->kindIndex) : "Nieznany";
            auto tierText = (isEditMode && *fs->tierIndex >= 0 && *fs->tierIndex < (int)fs->tierOptions->size())
                ? fs->tierOptions->at(*fs->tierIndex) : "Nieznany";

            auto leftCol = vbox({
                hbox({ text("Nazwa modelu: ") | size(WIDTH, EQUAL, 20), formContainer->ChildAt(0)->Render() }),
                hbox({ text("Rejestracja: ") | size(WIDTH, EQUAL, 20), formContainer->ChildAt(1)->Render() }),
                hbox({ text("Cena (PLN/dzień): ") | size(WIDTH, EQUAL, 20), formContainer->ChildAt(2)->Render() }),
                hbox({ text("Przebieg: ") | size(WIDTH, EQUAL, 20), formContainer->ChildAt(3)->Render() }),
                hbox({ text("Liczba miejsc: ") | size(WIDTH, EQUAL, 20), formContainer->ChildAt(4)->Render() }),
                separatorEmpty(),
                hbox({ text("Typ pojazdu: ") | size(WIDTH, EQUAL, 20), isEditMode ? text(kindText) | color(Color::GrayDark) : formContainer->ChildAt(5)->Render() }),
                hbox({ text("Klasa pojazdu: ") | size(WIDTH, EQUAL, 20), isEditMode ? text(tierText) | color(Color::GrayDark) : formContainer->ChildAt(6)->Render() }),

            });

            auto rightCol = vbox({

                formContainer->ChildAt(7)->Render(), // car
                formContainer->ChildAt(8)->Render(), // moto
                formContainer->ChildAt(9)->Render(), // truck
            });

            return vbox({
                text(isEditMode ? "Edytuj pojazd" : "Dodaj nowy pojazd") | bold,
                separatorEmpty(),
                hbox({
                    leftCol | hcenter | flex,

                    separator(),

                    rightCol | hcenter | flex
                }),
                separatorEmpty(),
                formContainer->ChildAt(10)->Render() | hcenter,
                text(*fs->resultMsg) | color(Color::Green) | hcenter
            }) | borderEmpty | vscroll_indicator | yframe | flex;
        });
    }
} // namespace

Component constructAdminDashboard(ApplicationState& state) {
    auto tab_index = std::make_shared<int>(0);
    auto tab_entries = std::make_shared<std::vector<std::string>>(std::vector<std::string>{
        " Lista pojazdów ",
        " Dodaj Pojazd "
    });
    auto tab_toggle = Toggle(tab_entries.get(), tab_index.get());

    // 1. Edit Vehicle logic
    auto editFs = std::make_shared<FormState>();
    auto selectedMenuIndex = std::make_shared<int>(0);
    auto selectedVehicleIndex = std::make_shared<int>(0);
    auto editVehicleLabels = std::make_shared<std::vector<std::string>>();
    auto filteredIndices = std::make_shared<std::vector<int>>();
    auto searchQuery = std::make_shared<std::string>();

    auto syncFormWithSelection = [&state, filteredIndices, selectedMenuIndex, selectedVehicleIndex, editFs]() {
        if (!filteredIndices->empty() && *selectedMenuIndex >= 0 && *selectedMenuIndex < static_cast<int>(filteredIndices->size())) {
            int actualIndex = (*filteredIndices)[*selectedMenuIndex];
            *selectedVehicleIndex = actualIndex;
            if (actualIndex >= 0 && actualIndex < static_cast<int>(state.loadedVehicles.size())) {
                if (state.loadedVehicles[actualIndex]) {
                    editFs->syncFromJSON(state.loadedVehicles[actualIndex]->toJSON());
                    *editFs->resultMsg = "";
                }
            }
        } else {
            editFs->clear();
            *editFs->resultMsg = "Brak wyników.";
        }
    };

    auto editForm = buildForm(state, editFs, true, selectedVehicleIndex);

    auto updateEditList = [&state, editVehicleLabels, filteredIndices, searchQuery, selectedMenuIndex, syncFormWithSelection]() {
        try {
            std::vector<std::string> newLabels;
            std::vector<int> newIndices;

            std::string q = *searchQuery;
            std::transform(q.begin(), q.end(), q.begin(), ::tolower);

            for (int i = 0; i < static_cast<int>(state.loadedVehicles.size()); ++i) {
                const auto& v = state.loadedVehicles[i];
                std::string rawName = v ? v->getName() : "Usunięty pojazd";
                if (rawName.length() > 50) rawName = rawName.substr(0, 50);

                std::string name = rawName + " (ID: " + std::to_string(v ? v->getId() : -1) + ")";

                std::string nameLower = name;
                std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);

                if (q.empty() || nameLower.find(q) != std::string::npos) {
                    newLabels.push_back(name);
                    newIndices.push_back(i);
                }
            }

            if (newLabels.empty()) {
                newLabels.push_back("Brak wyników");
            }

            *editVehicleLabels = std::move(newLabels);
            *filteredIndices = std::move(newIndices);

            if (*selectedMenuIndex < 0) *selectedMenuIndex = 0;
            if (!editVehicleLabels->empty() && *selectedMenuIndex >= static_cast<int>(editVehicleLabels->size())) {
                *selectedMenuIndex = static_cast<int>(editVehicleLabels->size() - 1);
            }

            syncFormWithSelection();
        } catch (...) {
            *editVehicleLabels = {"Błąd listy"};
            filteredIndices->clear();
            *selectedMenuIndex = 0;
        }
    };
    updateEditList();

    MenuOption editMenuOption;
    editMenuOption.on_change = syncFormWithSelection;
    auto editMenu = Menu(editVehicleLabels.get(), selectedMenuIndex.get(), editMenuOption);

    InputOption searchOpt;
    searchOpt.multiline = false;
    auto searchInput = Input(searchQuery.get(), "Szukaj...", searchOpt);

    auto editMenuScrolled = Renderer(editMenu, [editMenu] {
        return editMenu->Render() | vscroll_indicator | yframe | flex;
    });

    auto leftPanel = Container::Vertical({
        searchInput,
        editMenuScrolled
    });

    auto leftPanelRenderer = Renderer(leftPanel, [leftPanel] {
        return vbox({
            text("Wyszukaj (wciśnij '/'):") | dim,
            leftPanel->ChildAt(0)->Render(),
            separatorEmpty(),
            leftPanel->ChildAt(1)->Render()
        }) | size(WIDTH, EQUAL, 35);
    });

    auto leftPanelWithCatch = CatchEvent(leftPanelRenderer, [searchInput](Event e) {
        if (e == Event::Character('/') && !searchInput->Focused()) {
            searchInput->TakeFocus();
            return true;
        }
        return false;
    });

    auto editContainer = Container::Horizontal({
        leftPanelWithCatch,
        Maybe(editForm, [filteredIndices] { return !filteredIndices->empty(); })
    });

    auto lastSearchQuery = std::make_shared<std::string>();
    auto lastVehicleCount = std::make_shared<int>(state.loadedVehicles.size());

    auto editLogic = Renderer(editContainer, [&state, editContainer, updateEditList, searchQuery, lastSearchQuery, lastVehicleCount, filteredIndices, editForm, leftPanelWithCatch] {
        if (static_cast<int>(state.loadedVehicles.size()) != *lastVehicleCount || *searchQuery != *lastSearchQuery) {
            *lastVehicleCount = state.loadedVehicles.size();
            *lastSearchQuery = *searchQuery;
            updateEditList();
        }

        auto leftCol = leftPanelWithCatch->Render();
        if (filteredIndices->empty() || (*filteredIndices).empty()) {
            return hbox({ leftCol | border, text("Brak pojazdów.") | border }) | flex;
        }

        auto rightCol = editForm->Render();
        return hbox({ leftCol | border, rightCol | border }) | flex;
    });

    // 2. Add Vehicle logic
    auto addFs = std::make_shared<FormState>();
    auto addForm = buildForm(state, addFs, false);

    auto mainContainer = Container::Tab({
        editLogic,
        addForm
    }, tab_index.get());

    auto finalLayout = Container::Vertical({
        tab_toggle,
        mainContainer
    });

    return Renderer(finalLayout, [finalLayout, tab_toggle, mainContainer, tab_index, tab_entries] {
        return vbox({
            tab_toggle->Render() | hcenter,
            separator(),
            mainContainer->Render() | flex
        }) | size(WIDTH, GREATER_THAN, 130) | border | flex;
    });
}
