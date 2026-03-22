include(FetchContent)

FetchContent_Declare(
    imguizmo
    GIT_REPOSITORY https://github.com/CedricGuillemet/ImGuizmo.git
    GIT_TAG        master
)

FetchContent_MakeAvailable(imguizmo)

if(NOT TARGET imguizmo)
    add_library(imguizmo STATIC
        ${imguizmo_SOURCE_DIR}/ImGuizmo.cpp
    )

    target_include_directories(imguizmo PUBLIC
        ${imguizmo_SOURCE_DIR}
    )

    target_compile_definitions(imguizmo PRIVATE
        IMGUI_DEFINE_MATH_OPERATORS
    )

    target_link_libraries(imguizmo PUBLIC imgui)
endif()