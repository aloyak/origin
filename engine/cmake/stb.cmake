include(FetchContent)

FetchContent_Declare(
    stb_image
    GIT_REPOSITORY https://github.com/nothings/stb
    GIT_TAG        master
)

FetchContent_GetProperties(stb_image)
if(NOT stb_image_POPULATED)
    FetchContent_Populate(stb_image)
    
    add_library(stb_image INTERFACE)
    target_include_directories(stb_image INTERFACE ${stb_image_SOURCE_DIR})
endif()