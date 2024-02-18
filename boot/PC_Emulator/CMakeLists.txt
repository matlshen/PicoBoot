cmake_minimum_required(VERSION 3.14)

project(PC_Emulator LANGUAGES C CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core SerialPort)

include_directories(inc)
add_subdirectory(../../bootutil ${CMAKE_CURRENT_BINARY_DIR}/bootutil)

add_executable(PC_Emulator
    src/boot_uart.cpp
    src/main.cpp
)
target_link_libraries(PC_Emulator
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::SerialPort
    bootlib
)

include(GNUInstallDirs)
install(TARGETS PC_Emulator
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
