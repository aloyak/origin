include(FetchContent)

set(SOLOUD_BUILD_DEMOS OFF CACHE BOOL "" FORCE)
set(SOLOUD_GENERATE_GLUE OFF CACHE BOOL "" FORCE)
set(SOLOUD_C_API OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    soloud
    GIT_REPOSITORY https://github.com/jarikomppa/soloud.git
    GIT_TAG        master
    SOURCE_SUBDIR  contrib
    GIT_PROGRESS   TRUE
)

FetchContent_MakeAvailable(soloud)