include (FetchContent)

FetchContent_Declare(
    implot3d
    GIT_REPOSITORY https://github.com/brenocq/implot3d
    GIT_TAG        v0.4
)

FetchContent_MakeAvailable(implot3d)

if (NOT TARGET implot3d)
    add_library(implot3d STATIC
        ${implot3d_SOURCE_DIR}/implot3d.cpp
        ${implot3d_SOURCE_DIR}/implot3d_items.cpp
        ${implot3d_SOURCE_DIR}/implot3d_demo.cpp
        ${implot3d_SOURCE_DIR}/implot3d_meshes.cpp
    )

    target_link_libraries(implot3d PUBLIC imgui)

    target_include_directories(implot3d PUBLIC 
        ${implot3d_SOURCE_DIR} 
    )
endif()