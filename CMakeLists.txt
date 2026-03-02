cmake_minimum_required(VERSION 3.23)

set(PROJECT_NAME "origin")
set(PROJECT_VERS "0.1.0")

project(${PROJECT_NAME} VERSION ${PROJECT_VERS})
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(EMSCRIPTEN)
    message(STATUS "Emscripten build!")
    add_compile_options(-sUSE_SDL=2)
endif()

add_subdirectory(engine)
add_subdirectory(game)