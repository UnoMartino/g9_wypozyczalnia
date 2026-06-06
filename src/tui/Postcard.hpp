#pragma once

#include "ftxui/component/app.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include "../data/vehicle/Vehicle.hpp"

#include <functional>

using namespace ftxui;

// =====

// view component representing vehicle postcard
Component constructPostcardComponent(Vehicle* vehicle, std::function<void()> action);
