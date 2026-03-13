cmake_minimum_required(VERSION 3.23)

set(PROJECT_NAME "origin")
set(PROJECT_VERS "0.7.0")

project(${PROJECT_NAME} VERSION ${PROJECT_VERS})
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#file(REMOVE_RECURSE ${CMAKE_BINARY_DIR}/assets)
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})

if(EMSCRIPTEN)
    message(STATUS "Emscripten build!")
endif()

add_subdirectory(engine)
add_subdirectory(game)

if (NOT EMSCRIPTEN OR NOT DEFINED ORIGIN_NO_SANDBOX) # Emscripten may be added in the future
    add_subdirectory(sandbox)
endif()