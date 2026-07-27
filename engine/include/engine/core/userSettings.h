#pragma once

#include <fstream>
#include <string>
#include <nlohmann/json.hpp>

#include "engine/utils/path.h"

template <typename T>
class SettingsManager {
private:
    std::string filePath = "user/settings.json";
    T currentSettings;

public:
    SettingsManager(const std::string& path) : filePath(path) {}

    T& Get() {
        return currentSettings;
    }

    const T& Get() const {
        return currentSettings;
    }

    void Save() {
        nlohmann::json j = currentSettings;
        
        std::ofstream file(Path::resolve(filePath));
        if (file.is_open()) {
            file << j.dump(4);
            file.close();
        }
    }

    void Load() {
        std::ifstream file(Path::resolve(filePath));
        if (file.is_open()) {
            nlohmann::json j;
            file >> j;
            currentSettings = j.get<T>();
            file.close();
        }
    }
};