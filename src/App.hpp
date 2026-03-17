#pragma once

class Application {
private:
    bool isRunning;

public:
    Application();
    ~Application();

    void run();
    void shutdown();
};
