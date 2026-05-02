cmake_minimum_required(VERSION 3.20)
project(pinterest_server CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic)

find_package(Threads REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(OpenSSL REQUIRED)

# ----- Tum modul cpp'leri (main.cpp haric, hem server hem testte kullanilir) -----
set(PC_MODULE_SOURCES
    src/util.cpp
    src/db.cpp
    src/protocol.cpp
    src/session_cache.cpp
    src/auth.cpp
    src/session.cpp
    src/server.cpp
    src/media.cpp
)

# ----- server executable -----
add_executable(server
    src/main.cpp
    ${PC_MODULE_SOURCES}
)
target_include_directories(server PRIVATE src)
target_link_libraries(server PRIVATE
    Threads::Threads
    SQLite::SQLite3
    OpenSSL::Crypto
)

# ----- test_main executable -----
# tests/ klasoru varsa derle (yoksa atla, build kirilmamasin)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_main.cpp)
    add_executable(test_main
        tests/test_main.cpp
        ${PC_MODULE_SOURCES}
    )
    target_include_directories(test_main PRIVATE src)
    target_link_libraries(test_main PRIVATE
        Threads::Threads
        SQLite::SQLite3
        OpenSSL::Crypto
    )
endif()
