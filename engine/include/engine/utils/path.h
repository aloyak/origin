#pragma once

#include <filesystem>
#include <string>
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

    static void setBase(const std::filesystem::path& base) {
        s_base = base;
    }

    static std::filesystem::path getBase() {
        return s_base;
    }

    static std::filesystem::path resolve(const std::string& relative) {
        return s_base / relative;
    }

    static std::string toAssetsRelative(const std::string& inputPath) {
        if (inputPath.empty()) {
            return inputPath;
        }

        auto extractAssetsPath = [](const std::filesystem::path& path) -> std::string {
            std::filesystem::path result;
            bool foundAssets = false;

            for (const auto& part : path) {
                if (!foundAssets) {
                    if (part == "assets") {
                        foundAssets = true;
                        result /= part;
                    }
                    continue;
                }

                result /= part;
            }

            if (!foundAssets) {
                return "";
            }

            return result.generic_string();
        };

        const std::filesystem::path normalizedPath = std::filesystem::path(inputPath).lexically_normal();

        const std::string fromInput = extractAssetsPath(normalizedPath);
        if (!fromInput.empty()) {
            return fromInput;
        }

        if (normalizedPath.is_absolute()) {
            const std::string fromAbsolute = extractAssetsPath(std::filesystem::absolute(normalizedPath).lexically_normal());
            if (!fromAbsolute.empty()) {
                return fromAbsolute;
            }
        }

        return normalizedPath.generic_string();
    }
private:
    inline static std::filesystem::path s_base;
};