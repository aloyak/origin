file(GLOB_RECURSE SOURCES src/*.cpp)

add_executable(sandbox ${SOURCES})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources DESTINATION ${CMAKE_BINARY_DIR}/sandbox/)

target_link_libraries(sandbox PRIVATE engine)

# include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/imguizmo.cmake)

#target_link_libraries(sandbox PRIVATE imguizmo)