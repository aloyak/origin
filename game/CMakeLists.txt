set(GAME_NAME "game")

add_executable(${GAME_NAME} src/main.cpp)

file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR}/${GAME_NAME}/)

target_include_directories(${GAME_NAME}
    PUBLIC  include
    PRIVATE src
)

target_link_libraries(${GAME_NAME} PRIVATE engine)

if(WIN32)
    add_custom_command(TARGET ${GAME_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL2::SDL2>
            $<TARGET_FILE_DIR:${GAME_NAME}>
    )
endif()

if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    target_link_options(${GAME_NAME} PRIVATE
        -sUSE_SDL=2
        -sFULL_ES3=1
        -sUSE_WEBGL2=1
        -sALLOW_MEMORY_GROWTH=1
        -sINITIAL_MEMORY=536870912
        -sEXIT_RUNTIME=1
        -fexceptions
        --shell-file ${CMAKE_SOURCE_DIR}/game/web/shell.html
        --preload-file ${CMAKE_SOURCE_DIR}/assets@/assets --no-heap-copy --no-decode
        --use-preload-plugins
    )
    target_compile_options(${GAME_NAME} PRIVATE -fexceptions)
endif()