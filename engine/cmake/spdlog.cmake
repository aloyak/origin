include(FetchContent)

set(SPDLOG_BUILD_TESTS    OFF CACHE INTERNAL "")
set(SPDLOG_BUILD_EXAMPLES OFF CACHE INTERNAL "")
set(SPDLOG_BUILD_BENCH    OFF CACHE INTERNAL "")
set(SPDLOG_INSTALL        OFF CACHE INTERNAL "")

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog
    GIT_TAG        v1.17.0
)

FetchContent_MakeAvailable(spdlog)