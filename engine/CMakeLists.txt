file(GLOB_RECURSE SOURCES src/*.cpp)

add_library(engine STATIC ${SOURCES})

target_include_directories(engine
    PUBLIC  include
    PRIVATE src
)

if(EMSCRIPTEN)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/glm.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/spdlog.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/stb.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/assimp.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/nlohmann_json.cmake)

    # SDL2 and OpenGL come with Emscripten
    target_compile_options(engine PUBLIC -sUSE_SDL=2 -fexceptions)

    target_link_libraries(engine 
        PRIVATE glm::glm spdlog::spdlog stb_image assimp::assimp 
        PUBLIC  nlohmann_json::nlohmann_json
    )

else()
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/glad.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdl2.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/glm.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/spdlog.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/stb.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/assimp.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/nlohmann_json.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/imgui.cmake)

    target_link_libraries(engine 
        PRIVATE glad glm::glm spdlog::spdlog stb_image assimp::assimp
        PUBLIC  nlohmann_json::nlohmann_json SDL2::SDL2 imgui
    )
endif()