#pragma once

#include <map>
#include <string>
#include <memory>
#include "User.hpp"

// ====

class AuthManager {
private:
    std::map<std::string, std::unique_ptr<User>> m_users;

    User* m_currentUser = nullptr;

    size_t hashPassword(const std::string& password) const;

public:
    AuthManager() = default;

    bool signUp(const std::string& login, const std::string& password);
    bool signIn(const std::string& login, const std::string& password);
    void logout();
};
