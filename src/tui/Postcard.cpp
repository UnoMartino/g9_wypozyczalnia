#include "Postcard.hpp"
#include "ftxui/component/component_options.hpp"
#include "ftxui/dom/elements.hpp"

#include <functional>

// ====

Component constructPostcardComponent(Vehicle* vehicle, std::function<void()> action) {
    ButtonOption opt;

    opt.transform = [vehicle] (const EntryState& es) -> Element {
        Elements detail_lines;

        auto details_map = vehicle->getDetails();

        for (const auto& [key, value] : details_map) {
            detail_lines.push_back(text(key + ": " + value) | dim);
        }

        auto content = vbox({ vbox({
                hbox({
                    text("Klasa: " + std::string(tierToString(vehicle->getTier()))) | color(Color::Red),
                    filler(),
                    text("Cena: " + std::to_string(vehicle->getPrice()) + " zł / zaliczka")
                }) | flex | size(WIDTH, GREATER_THAN, 12),

                separator(),

                vbox(std::move(detail_lines)) | flex,

            }) | borderEmpty

        }) | border | size(WIDTH, GREATER_THAN, 90) | size(WIDTH, LESS_THAN, 120);

        if (es.focused) {
            content |= color(Color::Green);
        }

        return content;
    };

    return Button(vehicle->getName(), std::move(action), opt);
}
