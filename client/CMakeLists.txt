cmake_minimum_required(VERSION 3.20)
project(pinterest_client CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network Multimedia MultimediaWidgets)

add_executable(client
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.ui
    src/networkclient.cpp
)

target_include_directories(client PRIVATE
    src
    ../common
)

target_link_libraries(client PRIVATE
    Qt6::Widgets
    Qt6::Network
    Qt6::Multimedia
    Qt6::MultimediaWidgets
)
