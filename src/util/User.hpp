#pragma once

#include <string>
#include "../ext/json.hpp"

// ====

struct UserData {
    std::string email;
    size_t passwordHash;

    std::string firstName;
    std::string lastName;

    bool isAdmin = false;
};

inline void to_json(nlohmann::json& j, const UserData& u) {
    j = nlohmann::json{
        {"email", u.email},
        {"passwordHash", u.passwordHash},
        {"firstName", u.firstName},
        {"lastName", u.lastName},
        {"isAdmin", u.isAdmin}
    };
}

inline void from_json(const nlohmann::json& j, UserData& u) {
    j.at("email").get_to(u.email);
    j.at("passwordHash").get_to(u.passwordHash);
    if (j.contains("firstName")) j.at("firstName").get_to(u.firstName);
    if (j.contains("lastName")) j.at("lastName").get_to(u.lastName);
    if (j.contains("isAdmin")) j.at("isAdmin").get_to(u.isAdmin);
}

class User {
private:
   UserData m_data;

public:

    User(UserData data) : m_data(data) {}
    virtual ~User() = default;

    size_t getPasswordHash() { return m_data.passwordHash; }
    void setPasswordHash(size_t hash) { m_data.passwordHash = hash; }
    const std::string& getEmail() { return m_data.email; }
    const std::string& getFirstName() { return m_data.firstName; }
    const std::string& getLastName() { return m_data.lastName; }
    bool isAdmin() { return m_data.isAdmin; }

    const UserData& getData() const { return m_data; }
};
