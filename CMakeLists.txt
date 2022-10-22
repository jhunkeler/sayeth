cmake_minimum_required(VERSION 3.23)
project(sayeth C)

set(CMAKE_C_STANDARD 99)

add_executable(sayeth main.c)
set(DRIVER_DIR ${CMAKE_INSTALL_PREFIX}/share/${CMAKE_PROJECT_NAME})

option(DEV_ENABLE "Developer mode" OFF)
if(DEV_ENABLE)
    set(DRIVER_DIR ${CMAKE_SOURCE_DIR}/drivers)
endif()

configure_file(config.h.in config.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

install(TARGETS sayeth DESTINATION bin)
install(DIRECTORY drivers DESTINATION ${DRIVER_DIR})
