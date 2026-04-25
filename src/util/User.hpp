#pragma once

#include <string>

// ====

struct UserData {
    std::string login;
    size_t passwordHash;

    bool isAdmin = false;
};

class User {
private:
   UserData m_data;

public:

    User(UserData data) : m_data(data) {}
    virtual ~User() = default;

    size_t getPasswordHash() { return m_data.passwordHash; }

};
