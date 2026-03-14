#include <stdint.h>

enum VehicleKind {
    CAR,
    MOTORCYCLE,
    HEAVY_DUTY,
};

enum VehicleCategory {
    A = 1,
    B,
    C,
    D,
    E,
    F,
};

class Vehicle {
    public:

    int id;
    VehicleKind kind;
    VehicleCategory category;

    char* model_name;

    char* license_plate;
    uint32_t mileage;

    bool condition : 1;             // true - sprawny, false - trzeba zrobic przeglad
    bool is_rent   : 1;




    // vector<unix_time> historia_wypozyczen

    private:





};

class Car: public Vehicle {




};

class Motorcycle: public Vehicle {




};

class Truck: public Vehicle {




};
