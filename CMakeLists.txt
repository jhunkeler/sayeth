cmake_minimum_required(VERSION 3.17)
project(sayeth C)

set(CMAKE_C_STANDARD 99)
if (MSVC)
    # warning level 4 and all warnings as errors
    add_compile_options(/W4 /WX)
else()
    # lots of warnings and all warnings as errors
    add_compile_options(-Wall -Wextra -pedantic)
endif()

add_executable(sayeth main.c driver.c draw.c)
set(DRIVER_DIR ${CMAKE_INSTALL_PREFIX}/share/${CMAKE_PROJECT_NAME})

option(DEV_ENABLE "Developer mode" OFF)
if(DEV_ENABLE)
    set(DRIVER_DIR ${CMAKE_SOURCE_DIR}/drivers)
endif()

configure_file(config.h.in config.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

install(TARGETS sayeth DESTINATION bin)
install(DIRECTORY drivers DESTINATION ${DRIVER_DIR})
