include(FetchContent)

set(SDL_TEST        OFF CACHE INTERNAL "")
set(SDL_TESTS       OFF CACHE INTERNAL "")
set(SDL_INSTALL     OFF CACHE INTERNAL "")

FetchContent_Declare(
    sdl2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-2.32.8
)

FetchContent_MakeAvailable(sdl2)