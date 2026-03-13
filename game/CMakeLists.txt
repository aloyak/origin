set(GAME_NAME "game")

add_executable(${GAME_NAME} src/main.cpp)

file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR}/${GAME_NAME}/)

target_link_libraries(${GAME_NAME} PRIVATE engine)

if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    target_link_options(${GAME_NAME} PRIVATE
        -sUSE_SDL=2
        -sFULL_ES3=1
        -sUSE_WEBGL2=1
        -sALLOW_MEMORY_GROWTH=1
        -fexceptions
        --shell-file ${CMAKE_SOURCE_DIR}/game/web/shell.html
        --preload-file ${CMAKE_SOURCE_DIR}/assets@/assets
    )
    target_compile_options(${GAME_NAME} PRIVATE -fexceptions)
endif()