add_executable(game src/main.cpp)

target_link_libraries(game PRIVATE engine)

if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    target_link_options(game PRIVATE
        -sUSE_SDL=2
        -sFULL_ES2=1
        -sUSE_WEBGL2=1
        -sALLOW_MEMORY_GROWTH=1
        --shell-file ${CMAKE_SOURCE_DIR}/web/shell.html
    )
endif()