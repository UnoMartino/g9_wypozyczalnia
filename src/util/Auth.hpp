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

    void loadUsers();
    void saveUsers();

public:
    AuthManager() { loadUsers(); }
    ~AuthManager() { saveUsers(); }

    bool signUp(const std::string& email, const std::string& password, const std::string& firstName = "", const std::string& lastName = "");
    bool signIn(const std::string& email, const std::string& password);
    bool changePassword(const std::string& email, const std::string& oldPassword, const std::string& newPassword);
    void logout();

    User* getCurrentUser() { return m_currentUser; }
    bool isUserExists(const std::string& email) { return m_users.find(email) != m_users.end(); }
};
