cmake_minimum_required(VERSION 3.16)
project(wormhole-qt VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Find qrencode for QR code generation
find_package(PkgConfig REQUIRED)
pkg_check_modules(QRENCODE REQUIRED libqrencode)

set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/SendPage.cpp
    src/ReceivePage.cpp
    src/SettingsPage.cpp
    src/NavBar.cpp
)

set(HEADERS
    src/MainWindow.h
    src/SendPage.h
    src/ReceivePage.h
    src/SettingsPage.h
    src/NavBar.h
    src/DropArea.h
)

add_executable(wormhole-qt ${SOURCES} ${HEADERS})

# Compiler options
target_compile_options(wormhole-qt PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

# Include directories
target_include_directories(wormhole-qt PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${QRENCODE_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(wormhole-qt
    Qt6::Core
    Qt6::Widgets
    ${QRENCODE_LIBRARIES}
)

# Add compile definitions if needed
target_compile_definitions(wormhole-qt PRIVATE
    QT_DISABLE_DEPRECATED_BEFORE=0x060000
)

# Set properties for better debugging
set_target_properties(wormhole-qt PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

install(TARGETS wormhole-qt RUNTIME DESTINATION bin)
