#include "OrderSummary.hpp"

#include <memory>
#include <functional>
#include <iomanip>
#include <sstream>

#include "ftxui/component/component.hpp"
#include "ftxui/dom/elements.hpp"

// ====

Component constructOrderSummary(ApplicationState& state, std::shared_ptr<Order> order, std::function<void()> onBack) {

    auto formatPrice = [](double price) {
        std::ostringstream stream;
        stream << std::fixed << std::setprecision(2) << price;
        return stream.str();
    };

    auto btn = Button("Powrót do strony głównej", onBack, ButtonOption::Ascii());

    return Renderer(btn, [&state, order, formatPrice, btn] {
        
        auto content = vbox({
            text("Dziękujemy za złożenie zamówienia!") | bold | color(Color::Green) | hcenter,
            separator(),
            vbox({
                hbox({ text("Zamawiający: "), filler(), text(order->firstName + " " + order->lastName) }),
                hbox({ text("E-mail: "), filler(), text(order->email) }),
                separator(),
                hbox({ text("Termin: "), filler(), text(order->rentRange.toString()) }),
                hbox({ text("Ubezpieczenie: "), filler(), text(order->wantsInsurance ? "Tak" : "Nie") }),
                hbox({ text("Limit przebiegu: "), filler(), text(order->mileageOptions[order->mileageTier]) }),
                separator(),
                hbox({ text("Łączna cena: "), filler(), text(formatPrice(order->price) + " PLN") | bold | color(Color::Yellow) }),
            }) | borderEmpty,
            
            filler(),
            separator(),
            btn->Render() | hcenter
        }) | border | size(WIDTH, EQUAL, 60) | center;

        return content;
    });
}
