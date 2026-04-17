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

    void playSound(const char* filename);
    SoundHandle playSound(const std::string& filename, float volume = 1.0f, bool loop = false);

    void stopSound(SoundHandle handle);
    void stopAllSounds();

    void setSoundVolume(SoundHandle handle, float volume);
    void setGlobalVolume(float volume);

    void setListenerPosition(const Vec3& position);
    void clearListenerPosition();

    bool hasListenerPosition() const { return m_hasListenerPosition; }
    Vec3 getListenerPosition() const { return m_listenerPosition; }
    
    bool isAvailable() const;

    static AudioSystem* getActive();

private:
    SoLoud::Soloud* m_soloud = nullptr;
    std::unordered_map<std::string, std::unique_ptr<SoLoud::Wav>> m_soundCache;
    Vec3 m_listenerPosition = Vec3(0.0f, 0.0f, 0.0f);
    bool m_hasListenerPosition = false;
};