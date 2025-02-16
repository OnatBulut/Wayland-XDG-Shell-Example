cmake_minimum_required(VERSION 3.29)
project(Wayland_XDG_Shell_Example C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")

find_package(PkgConfig REQUIRED)
pkg_check_modules(WAYLAND_CLIENT REQUIRED wayland-client)

include_directories(${WAYLAND_CLIENT_INCLUDE_DIRS})
link_directories(${WAYLAND_CLIENT_LIBRARY_DIRS})

set(PROTOCOLS_DIR ${CMAKE_CURRENT_BINARY_DIR}/protocols)
file(MAKE_DIRECTORY ${PROTOCOLS_DIR})

set(WAYLAND_PROTOCOLS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/protocols)

add_custom_command(
        OUTPUT ${PROTOCOLS_DIR}/xdg-shell-client-protocol.h ${PROTOCOLS_DIR}/xdg-shell-protocol.c
        COMMAND wayland-scanner client-header ${WAYLAND_PROTOCOLS_DIR}/xdg-shell.xml ${PROTOCOLS_DIR}/xdg-shell-client-protocol.h
        COMMAND wayland-scanner private-code ${WAYLAND_PROTOCOLS_DIR}/xdg-shell.xml ${PROTOCOLS_DIR}/xdg-shell-protocol.c
        DEPENDS ${WAYLAND_PROTOCOLS_DIR}/xdg-shell.xml
)

add_library(xdg-shell-protocols STATIC ${PROTOCOLS_DIR}/xdg-shell-protocol.c)
add_executable(Wayland_XDG_Shell_Example main.c)
add_dependencies(Wayland_XDG_Shell_Example xdg-shell-protocols)

target_include_directories(Wayland_XDG_Shell_Example PRIVATE ${PROTOCOLS_DIR})
target_link_libraries(Wayland_XDG_Shell_Example m ${WAYLAND_CLIENT_LIBRARIES} xdg-shell-protocols)
