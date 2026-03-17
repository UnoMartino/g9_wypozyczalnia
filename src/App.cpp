#include "App.hpp"

Application::Application() : isRunning(false) {}

Application::~Application() {}

void Application::run() {

    isRunning = true;
    while (isRunning) {

    }
}

void Application::shutdown() {
    isRunning = false;
}
