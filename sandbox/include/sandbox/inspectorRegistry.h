#pragma once

#include <typeindex>
#include <functional>
#include <unordered_map>

#include <imgui.h>

class InspectorRegistry {
public:
    template<typename T>
    static void registerComponent(std::function<void(T*)> fn) {
        s_drawers[typeid(T)] = [fn](void* comp) {
            fn(static_cast<T*>(comp));
        };
    }

    static void draw(std::type_index type, void* comp) {
        auto it = s_drawers.find(type);
        if (it != s_drawers.end())
            it->second(comp);
        else
            ImGui::TextDisabled("No inspector registered");
    }

private:
    inline static std::unordered_map<std::type_index, std::function<void(void*)>> s_drawers;
};

void registerDefaultInspectors();