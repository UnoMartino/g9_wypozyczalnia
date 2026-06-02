#pragma once

#include "ftxui/component/app.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <functional>

#include "../data/vehicle/Vehicle.hpp"

#include "../data/Order.hpp"

using namespace ftxui;

// =====

Component constructConfigurationForm(Vehicle* vehicle, std::function<void(std::shared_ptr<Order>)> action);
