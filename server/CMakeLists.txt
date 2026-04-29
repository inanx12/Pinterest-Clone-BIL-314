cmake_minimum_required(VERSION 3.20)
project(pinterest_server CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic)

find_package(Threads REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(OpenSSL REQUIRED)

add_executable(server
    src/main.cpp
)

target_include_directories(server PRIVATE src)
target_link_libraries(server PRIVATE
    Threads::Threads
    SQLite::SQLite3
    OpenSSL::Crypto
)
