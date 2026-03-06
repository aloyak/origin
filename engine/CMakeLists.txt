file(GLOB_RECURSE SOURCES src/*.cpp)

add_library(engine STATIC ${SOURCES})

target_include_directories(engine
    PUBLIC  include
    PRIVATE src
)

if(EMSCRIPTEN)
    target_compile_options(engine PUBLIC -sUSE_SDL=2)

else()
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/glad.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdl2.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/glm.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/spdlog.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/stb.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/assimp.cmake)

    target_link_libraries(engine PRIVATE glad SDL2::SDL2 glm::glm spdlog::spdlog stb_image assimp::assimp)

endif()