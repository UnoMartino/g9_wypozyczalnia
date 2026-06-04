#include "Auth.hpp"
#include "User.hpp"

#include <iostream>
#include <memory>
#include <fstream>
#include "../ext/json.hpp"

// ====

size_t AuthManager::hashPassword(const std::string& password) const {
    return std::hash<std::string>{}(password);
} // AuthManager::hashPassword


bool AuthManager::signUp(const std::string& email, const std::string& password, const std::string& firstName, const std::string& lastName) {
    if (m_users.find(email) != m_users.end()) {
        std::cerr << "[AUTH] user with email " << email << " already exists\n";
        return false;
    }

    UserData data;
    data.email = email;
    data.passwordHash = hashPassword(password);
    data.firstName = firstName;
    data.lastName = lastName;

    m_users[email] = std::make_unique<User>(std::move(data));
    saveUsers();

    return true;
} // AuthManager::signUp


bool AuthManager::signIn(const std::string& email, const std::string& password) {
    if (m_users.find(email) == m_users.end()) {
        std::cerr << "[AUTH] user with email " << email << " not found\n";
        return false;
    }

    if (m_users[email]->getPasswordHash() != hashPassword(password)) {
        std::cerr << "[AUTH] invalid password for user " << email << "\n";
        return false;
    }

    m_currentUser = m_users[email].get();
    return true;
} // AuthManager::signIn


void AuthManager::logout() {
    if (m_currentUser) m_currentUser = nullptr;
} // AuthManager::logout


void AuthManager::loadUsers() {
    {
        std::ifstream file("users.json");
        if (file.is_open()) {
            nlohmann::json j;
            try {
                file >> j;
                for (const auto& item : j) {
                    UserData data;
                    from_json(item, data);
                    m_users[data.email] = std::make_unique<User>(std::move(data));
                }
            } catch (...) {
                std::cerr << "[AUTH] Error parsing users.json\n";
            }
        }
    }

    // Default admin fallback
    if (m_users.find("admin@admin.com") == m_users.end()) {
        UserData adminData;
        adminData.email = "admin@admin.com";
        adminData.passwordHash = hashPassword("admin");
        adminData.firstName = "Admin";
        adminData.lastName = "Admin";
        adminData.isAdmin = true;
        m_users["admin@admin.com"] = std::make_unique<User>(std::move(adminData));
        saveUsers();
    }
}

void AuthManager::saveUsers() {
    nlohmann::json j = nlohmann::json::array();
    for (const auto& [email, user] : m_users) {
        nlohmann::json user_json;
        to_json(user_json, user->getData());
        j.push_back(user_json);
    }

    std::ofstream file("users.json");
    if (file.is_open()) {
        file << j.dump(4);
    }
}
