#pragma once
#include <string>
#include <optional>
#include "db.hpp"

namespace pc {
struct User {
    int id;
    std::string username;
};

struct RegisterResult {
    bool success;
    std::string token;
    std::string error;
    int error_code;
};

RegisterResult register_user(Database& db, const std::string& username, const std::string& password);

struct LoginResult {
    bool success;
    std::string token;
    std::string error;
    int error_code;
};

LoginResult login_user(Database& db, const std::string& username, const std::string& password);
std::optional<User> validate_token(Database& db, const std::string& token);
bool logout(Database& db, const std::string& token);
}
