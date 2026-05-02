cmake_minimum_required(VERSION 3.20)
project(pinterest_client CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network Multimedia MultimediaWidgets)

add_executable(client
    src/main.cpp
    src/networkclient.cpp
    src/networkclient.hpp
    src/apiclient.cpp
    src/apiclient.hpp
    src/mediainfo.cpp
    src/mediainfo.hpp
    src/loginwindow.cpp
    src/loginwindow.hpp
    src/mainwindow.cpp
    src/mainwindow.hpp
    src/mediacard.cpp
    src/mediacard.hpp
    src/imageviewer.cpp
    src/imageviewer.hpp
    src/videoviewer.cpp
    src/videoviewer.hpp
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

# Windows: client.exe build edildikten sonra Qt DLL'lerini yanına kopyala
if(WIN32)
    find_program(WINDEPLOYQT_EXE windeployqt
        HINTS "${Qt6_DIR}/../../../bin")
    if(WINDEPLOYQT_EXE)
        add_custom_command(TARGET client POST_BUILD
            COMMAND "${WINDEPLOYQT_EXE}"
                    --no-translations
                    --no-system-d3d-compiler
                    "$<TARGET_FILE:client>"
            COMMENT "Running windeployqt..."
        )
    endif()
endif()
