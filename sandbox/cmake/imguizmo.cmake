include(FetchContent)

FetchContent_Declare(
    imguizmo
    GIT_REPOSITORY https://github.com/CedricGuillemet/ImGuizmo.git
    GIT_TAG        1.83
)

FetchContent_MakeAvailable(imguizmo)

set(IMGUIZMO_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)