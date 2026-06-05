#include "../../ext/json.hpp"

#include <iostream>
#include <fstream>

using json = nlohmann::json;

json loadFile(std::string path) {
    std::fstream f(path);

    if (!f.is_open()) {
        std::cerr << "File not found\n";
        return json::array();
    }

    if (f.peek() == std::ifstream::traits_type::eof()) {
        return json::array();
    }

    try {
        return json::parse(f);
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "Parse error: " << e.what() << "\n";
        return json::array();
    }
}

void saveFile(std::string path, std::string buffer) {




}
