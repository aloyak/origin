#pragma once

#include <filesystem>
#include <SDL2/SDL.h>

// Utility class to fix relative paths
// Shouldn't be used outside of the engine

class Path {
public:
    static void init() {
        char* base = SDL_GetBasePath();
        s_base = base ? base : "./";
        SDL_free(base);
    }

    static std::filesystem::path resolve(const std::string& relative) {
        return s_base / relative;
    }
private:
    inline static std::filesystem::path s_base;
};