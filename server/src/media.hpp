#pragma once
#include <string>
#include <vector>
#include "db.hpp"

namespace pc {
struct UploadResult {
    bool success;
    std::string media_id;
    int error_code;
    std::string error_msg;
};

UploadResult handle_upload(Database& db, int user_id);
}
