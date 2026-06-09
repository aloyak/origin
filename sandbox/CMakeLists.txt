file(GLOB_RECURSE SOURCES src/*.cpp)

add_executable(sandbox ${SOURCES})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources DESTINATION ${CMAKE_BINARY_DIR}/sandbox/)

target_include_directories(sandbox
    PUBLIC  include
    PRIVATE src
)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/pfd.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/imguizmo.cmake)

target_link_libraries(sandbox PRIVATE engine imguizmo pfd)
target_link_libraries(imguizmo PRIVATE imgui)

if(WIN32)
    add_custom_command(TARGET sandbox POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL2::SDL2>
            $<TARGET_FILE_DIR:sandbox>
    )
endif()