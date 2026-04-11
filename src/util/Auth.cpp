#include "Auth.hpp"
#include "User.hpp"

#include <iostream>
#include <memory>


// ====

size_t AuthManager::hashPassword(const std::string& password) const {
    size_t hash = 14695981039346656037ULL; // FNV offset basis
    for (char c : password) {
        hash ^= static_cast<size_t>(c);
        hash *= 1099511628211ULL;      // FNV prime
    }
    return hash;
} // AuthManager::hashPassword


bool AuthManager::signUp(const std::string& login, const std::string& password) {
    if (m_users.find(login) != m_users.end()) {
        std::cerr << "[AUTH] user with login " << login << " already exists\n";
        return false;
    }

    UserData data;
    data.login = login;
    data.passwordHash = hashPassword(password);

    m_users[login] = std::make_unique<User>(std::move(data));

    return true;
} // AuthManager::signUp


bool AuthManager::signIn(const std::string& login, const std::string& password) {
    if (m_users.find(login) == m_users.end()) {
        std::cerr << "[AUTH] user with login " << login << " not found\n";
        return false;
    }

    if (m_users[login]->getPasswordHash() != hashPassword(password)) {
        std::cerr << "[AUTH] invalid password for user " << login << "\n";
        return false;
    }

    m_currentUser = m_users[login].get();
    return true;
} // AuthManager::signIn


void AuthManager::logout() {
    if (m_currentUser) m_currentUser = nullptr;
} // AuthManager::logout
