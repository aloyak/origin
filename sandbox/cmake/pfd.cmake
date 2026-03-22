include(FetchContent)

FetchContent_Declare(
    pfd
    GIT_REPOSITORY https://github.com/samhocevar/portable-file-dialogs
    GIT_TAG        0.1.0
)

FetchContent_GetProperties(pfd)
if(NOT pfd_POPULATED)
    FetchContent_Populate(pfd)
    
    add_library(pfd INTERFACE)
    target_include_directories(pfd INTERFACE ${pfd_SOURCE_DIR})
endif()