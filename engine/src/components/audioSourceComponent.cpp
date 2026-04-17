#include "engine/components/audioSourceComponent.h"
#include "engine/components/entity.h"

#include "engine/utils/path.h"
#include <nlohmann/json.hpp>

#include <algorithm>
#include <cmath>

namespace {
float clampVolume(float volume) {
    return std::clamp(volume, 0.0f, 8.0f);
}

float clampRadius(float radius) {
    return std::max(radius, 0.0f);
}

float clampFalloffExponent(float exponent) {
    return std::clamp(exponent, 0.01f, 16.0f);
}
}

AudioSourceComponent::AudioSourceComponent(const std::string& soundPath, bool playOnStart, bool looping, float volume)
    : m_playOnStart(playOnStart), m_looping(looping), m_volume(clampVolume(volume)) {
    setSoundPath(soundPath);
}

void AudioSourceComponent::update(float /*dt*/) {
    if (!isEnabled) {
        return;
    }

    if (m_playOnStart && !m_started) {
        m_started = true;
        play();
    }

    updateAttenuatedVolume();
}

std::unique_ptr<Component> AudioSourceComponent::clone() const {
    auto copy = std::make_unique<AudioSourceComponent>(m_soundPath, m_playOnStart, m_looping, m_volume);
    copy->m_radius = m_radius;
    copy->m_useFalloff = m_useFalloff;
    copy->m_falloffExponent = m_falloffExponent;
    copy->isEnabled = isEnabled;
    return copy;
}

void AudioSourceComponent::serialize(nlohmann::json& j) const {
    j["type"] = "AudioSourceComponent";
    j["sound"] = m_soundPath;
    j["playOnStart"] = m_playOnStart;
    j["looping"] = m_looping;
    j["volume"] = m_volume;
    j["radius"] = m_radius;
    j["useFalloff"] = m_useFalloff;
    j["falloffExponent"] = m_falloffExponent;
}

void AudioSourceComponent::deserialize(const nlohmann::json& j) {
    setSoundPath(j.value("sound", std::string("")));
    setPlayOnStart(j.value("playOnStart", false));
    setLooping(j.value("looping", false));
    setVolume(j.value("volume", 1.0f));
    setRadius(j.value("radius", 20.0f));
    setUseFalloff(j.value("useFalloff", true));
    setFalloffExponent(j.value("falloffExponent", 1.0f));

    m_started = false;
}

void AudioSourceComponent::play() {
    AudioSystem* audio = AudioSystem::getActive();
    if (audio == nullptr || !audio->isAvailable() || m_soundPath.empty()) {
        return;
    }

    if (m_handle != 0) {
        audio->stopSound(m_handle);
        m_handle = 0;
    }

    m_handle = audio->playSound(m_soundPath, m_volume, m_looping);
    updateAttenuatedVolume();
}

void AudioSourceComponent::stop() {
    AudioSystem* audio = AudioSystem::getActive();
    if (audio == nullptr || m_handle == 0) {
        return;
    }

    audio->stopSound(m_handle);
    m_handle = 0;
}

void AudioSourceComponent::setSoundPath(const std::string& soundPath) {
    m_soundPath = Path::toAssetsRelative(soundPath);
}

void AudioSourceComponent::setLooping(bool looping) {
    m_looping = looping;

    AudioSystem* audio = AudioSystem::getActive();
    if (audio == nullptr || m_handle == 0) {
        return;
    }

    audio->stopSound(m_handle);
    m_handle = audio->playSound(m_soundPath, m_volume, m_looping);
}

void AudioSourceComponent::setVolume(float volume) {
    m_volume = clampVolume(volume);

    updateAttenuatedVolume();
}

void AudioSourceComponent::setRadius(float radius) {
    m_radius = clampRadius(radius);

    updateAttenuatedVolume();
}

void AudioSourceComponent::setFalloffExponent(float exponent) {
    m_falloffExponent = clampFalloffExponent(exponent);

    updateAttenuatedVolume();
}

void AudioSourceComponent::updateAttenuatedVolume() {
    AudioSystem* audio = AudioSystem::getActive();
    if (audio == nullptr || m_handle == 0) {
        return;
    }

    float attenuation = 1.0f;
    if (m_radius > 0.0f && audio->hasListenerPosition() && entity != nullptr) {
        const Vec3 sourceToListener = entity->transform.position - audio->getListenerPosition();
        const float distance = sourceToListener.length();
        if (distance >= m_radius) {
            attenuation = 0.0f;
        } else if (m_useFalloff) {
            const float normalized = std::clamp(1.0f - (distance / m_radius), 0.0f, 1.0f);
            attenuation = std::pow(normalized, m_falloffExponent);
        }
    }

    audio->setSoundVolume(m_handle, m_volume * attenuation);
}

void AudioSourceComponent::setUseFalloff(bool useFalloff) {
    m_useFalloff = useFalloff;

    updateAttenuatedVolume();
}
