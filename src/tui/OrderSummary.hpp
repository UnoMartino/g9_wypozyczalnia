#pragma once

#include "View.hpp"
#include "ftxui/component/app.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <functional>

#include "../data/vehicle/Vehicle.hpp"
#include "../data/Order.hpp"
#include "../Application.hpp"

using namespace ftxui;

Component constructOrderSummary(ApplicationState& state, std::shared_ptr<Order> order, std::function<void()> onBack);
