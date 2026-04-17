#pragma once

#include "engine/components/component.h"

class ListenerComponent : public Component {
public:
	std::unique_ptr<Component> clone() const override;

	void serialize(nlohmann::json& j) const override;
	void deserialize(const nlohmann::json& j) override;
};