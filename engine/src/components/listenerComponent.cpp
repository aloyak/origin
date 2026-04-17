#include "engine/components/listenerComponent.h"

#include <nlohmann/json.hpp>

std::unique_ptr<Component> ListenerComponent::clone() const {
    auto copy = std::make_unique<ListenerComponent>();
    copy->isEnabled = isEnabled;
    return copy;
}

void ListenerComponent::serialize(nlohmann::json& j) const {
    j["type"] = "ListenerComponent";
}

void ListenerComponent::deserialize(const nlohmann::json& /*j*/) {
}
