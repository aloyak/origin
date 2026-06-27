#pragma once

#include "engine/core/math.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace SoLoud {
class Soloud;
class Wav;
}

class AudioSystem {
public:
    using SoundHandle = unsigned int;

    AudioSystem();
    ~AudioSystem();

    void update(float deltaTime);

    void playSound(const char* filename);
    SoundHandle playSound(const std::string& filename, float volume = 1.0f, bool loop = false);

    void stopSound(SoundHandle handle);
    void stopAllSounds();

    void setSoundVolume(SoundHandle handle, float volume);

    void fadeSoundVolume(SoundHandle handle, float targetVolume, float durationSeconds);
    void fadeOutAndStop(SoundHandle handle, float durationSeconds);

    void setGlobalVolume(float volume);
    float getGlobalVolume() const { return m_globalVolume; }

    bool isHandleValid(SoundHandle handle) const;

    void setListenerPosition(const Vec3& position);
    void clearListenerPosition();

    bool hasListenerPosition() const { return m_hasListenerPosition; }
    Vec3 getListenerPosition() const { return m_listenerPosition; }

    bool isAvailable() const;

    static AudioSystem* getActive();

private:
    struct LiveSound {
        float baseVolume = 1.0f;
        bool fading = false;
        float fadeFrom = 1.0f;
        float fadeTo = 1.0f;
        float fadeDuration = 0.0f;
        float fadeElapsed = 0.0f;
        bool stopOnFadeComplete = false;
    };

    void applyEffectiveVolume(SoundHandle handle, const LiveSound& sound);
    void pruneFinishedSounds();

    SoLoud::Soloud* m_soloud = nullptr;
    std::unordered_map<std::string, std::unique_ptr<SoLoud::Wav>> m_soundCache;

    std::unordered_map<SoundHandle, LiveSound> m_liveSounds;
    float m_globalVolume = 1.0f;

    Vec3 m_listenerPosition = Vec3(0.0f, 0.0f, 0.0f);
    bool m_hasListenerPosition = false;
};