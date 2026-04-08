#pragma once

#include "../../ext/json.hpp"

// ====

using json = nlohmann::json;

json loadFile(std::string path);
