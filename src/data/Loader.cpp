#include "../../ext/json.hpp"

#include <iostream>
#include <fstream>

using json = nlohmann::json;

json loadFile(std::string path) {

    std::fstream f(path);

    if (!f.is_open()) {
        std::cerr << "File not found\n";
        return false;
    }

    json data = json::parse(f);

    return data;
}

void saveFile(std::string path, std::string buffer) {




}
