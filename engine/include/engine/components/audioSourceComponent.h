#pragma once

#include "engine/audio/audio.h"

#include "engine/components/component.h"

#include <string>

class AudioSourceComponent : public Component {
public:
	AudioSourceComponent(const std::string& soundPath = "", bool playOnStart = false, bool looping = false, float volume = 1.0f);

	void update(float dt) override;

	std::unique_ptr<Component> clone() const override;

	void serialize(nlohmann::json& j) const override;
	void deserialize(const nlohmann::json& j) override;

	void play();
	void stop();

	void setSoundPath(const std::string& soundPath);
	const std::string& getSoundPath() const { return m_soundPath; }

	void setPlayOnStart(bool playOnStart) { m_playOnStart = playOnStart; }
	bool getPlayOnStart() const { return m_playOnStart; }

	void setLooping(bool looping);
	bool getLooping() const { return m_looping; }

	void setVolume(float volume);
	float getVolume() const { return m_volume; }

	void setRadius(float radius);
	float getRadius() const { return m_radius; }

	void setUseFalloff(bool useFalloff);
	bool getUseFalloff() const { return m_useFalloff; }

	void setFalloffExponent(float exponent);
	float getFalloffExponent() const { return m_falloffExponent; }

private:
	void updateAttenuatedVolume();

	std::string m_soundPath;
	bool m_playOnStart = false;
	bool m_looping = false;
	bool m_started = false;
	float m_volume = 1.0f;
	float m_radius = 20.0f;
	bool m_useFalloff = true;
	float m_falloffExponent = 1.0f;
	AudioSystem::SoundHandle m_handle = 0;
};