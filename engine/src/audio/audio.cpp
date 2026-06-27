#include "engine/audio/audio.h"

#include "engine/utils/path.h"

#include <soloud.h>
#include <soloud_wav.h>

#include <spdlog/spdlog.h>

#include <algorithm>
#include <vector>

namespace {
AudioSystem* g_activeAudioSystem = nullptr;

float clampVolume(float value) {
    return std::clamp(value, 0.0f, 8.0f);
}
}

AudioSystem::AudioSystem() {
    g_activeAudioSystem = this;

    m_soloud = new SoLoud::Soloud();

    const int initResult = m_soloud->init();
    if (initResult != SoLoud::SO_NO_ERROR) {
        spdlog::error("AudioSystem init failed with SoLoud error code {}", initResult);
        delete m_soloud;
        m_soloud = nullptr;
    }
}

AudioSystem::~AudioSystem() {
    stopAllSounds();
    m_soundCache.clear();

    if (m_soloud != nullptr) {
        m_soloud->deinit();
        delete m_soloud;
        m_soloud = nullptr;
    }

    if (g_activeAudioSystem == this) {
        g_activeAudioSystem = nullptr;
    }
}

void AudioSystem::update(float deltaTime) {
    if (m_soloud == nullptr || m_liveSounds.empty()) {
        return;
    }

    std::vector<SoundHandle> toStop;

    for (auto& [handle, sound] : m_liveSounds) {
        if (!sound.fading) {
            continue;
        }

        sound.fadeElapsed += deltaTime;
        const float t = sound.fadeDuration > 0.0f
            ? std::clamp(sound.fadeElapsed / sound.fadeDuration, 0.0f, 1.0f)
            : 1.0f;

        sound.baseVolume = std::lerp(sound.fadeFrom, sound.fadeTo, t);
        applyEffectiveVolume(handle, sound);

        if (t >= 1.0f) {
            sound.fading = false;
            if (sound.stopOnFadeComplete) {
                toStop.push_back(handle);
            }
        }
    }

    for (SoundHandle handle : toStop) {
        stopSound(handle);
    }

    pruneFinishedSounds();
}

void AudioSystem::playSound(const char* filename) {
    if (filename == nullptr || filename[0] == '\0') {
        return;
    }

    playSound(std::string(filename), 1.0f, false);
}

AudioSystem::SoundHandle AudioSystem::playSound(const std::string& filename, float volume, bool loop) {
    if (m_soloud == nullptr || filename.empty()) {
        return 0;
    }

    const std::string normalizedPath = Path::toAssetsRelative(filename);
    const std::string cacheKey = normalizedPath.empty() ? filename : normalizedPath;

    auto cached = m_soundCache.find(cacheKey);
    if (cached == m_soundCache.end()) {
        auto wav = std::make_unique<SoLoud::Wav>();
        const std::string resolvedPath = Path::resolve(cacheKey).string();
        const int loadResult = wav->load(resolvedPath.c_str());
        if (loadResult != SoLoud::SO_NO_ERROR) {
            spdlog::error("AudioSystem failed to load sound '{}' with SoLoud error code {}", cacheKey, loadResult);
            return 0;
        }

        cached = m_soundCache.emplace(cacheKey, std::move(wav)).first;
    }

    const float startVolume = clampVolume(volume * m_globalVolume);

    const SoLoud::handle handle = m_soloud->play(*cached->second, startVolume, 0.0f, false, 0);
    if (handle == 0) {
        return 0;
    }

    m_soloud->setLooping(handle, loop ? 1 : 0);

    const SoundHandle soundHandle = static_cast<SoundHandle>(handle);

    LiveSound sound;
    sound.baseVolume = volume;
    m_liveSounds[soundHandle] = sound;

    return soundHandle;
}

void AudioSystem::stopSound(SoundHandle handle) {
    if (m_soloud == nullptr || handle == 0) {
        return;
    }

    m_soloud->stop(static_cast<SoLoud::handle>(handle));
    m_liveSounds.erase(handle);
}

void AudioSystem::stopAllSounds() {
    if (m_soloud != nullptr) {
        m_soloud->stopAll();
    }
    m_liveSounds.clear();
}

void AudioSystem::setSoundVolume(SoundHandle handle, float volume) {
    auto it = m_liveSounds.find(handle);
    if (it == m_liveSounds.end()) {
        return;
    }

    it->second.fading = false;
    it->second.baseVolume = volume;
    applyEffectiveVolume(handle, it->second);
}

void AudioSystem::fadeSoundVolume(SoundHandle handle, float targetVolume, float durationSeconds) {
    auto it = m_liveSounds.find(handle);
    if (it == m_liveSounds.end()) {
        return;
    }

    if (durationSeconds <= 0.0f) {
        setSoundVolume(handle, targetVolume);
        return;
    }

    it->second.fading = true;
    it->second.fadeFrom = it->second.baseVolume;
    it->second.fadeTo = targetVolume;
    it->second.fadeDuration = durationSeconds;
    it->second.fadeElapsed = 0.0f;
    it->second.stopOnFadeComplete = false;
}

void AudioSystem::fadeOutAndStop(SoundHandle handle, float durationSeconds) {
    auto it = m_liveSounds.find(handle);
    if (it == m_liveSounds.end()) {
        return;
    }

    if (durationSeconds <= 0.0f) {
        stopSound(handle);
        return;
    }

    it->second.fading = true;
    it->second.fadeFrom = it->second.baseVolume;
    it->second.fadeTo = 0.0f;
    it->second.fadeDuration = durationSeconds;
    it->second.fadeElapsed = 0.0f;
    it->second.stopOnFadeComplete = true;
}

void AudioSystem::setGlobalVolume(float volume) {
    m_globalVolume = volume;

    if (m_soloud == nullptr) {
        return;
    }

    for (auto& [handle, sound] : m_liveSounds) {
        applyEffectiveVolume(handle, sound);
    }
}

bool AudioSystem::isHandleValid(SoundHandle handle) const {
    if (m_soloud == nullptr || handle == 0) {
        return false;
    }
    return m_soloud->isValidVoiceHandle(static_cast<SoLoud::handle>(handle));
}

void AudioSystem::applyEffectiveVolume(SoundHandle handle, const LiveSound& sound) {
    if (m_soloud == nullptr) {
        return;
    }

    const float effectiveVolume = clampVolume(sound.baseVolume * m_globalVolume);
    m_soloud->setVolume(static_cast<SoLoud::handle>(handle), effectiveVolume);
}

void AudioSystem::pruneFinishedSounds() {
    if (m_soloud == nullptr) {
        return;
    }

    for (auto it = m_liveSounds.begin(); it != m_liveSounds.end();) {
        if (!m_soloud->isValidVoiceHandle(static_cast<SoLoud::handle>(it->first))) {
            it = m_liveSounds.erase(it);
        } else {
            ++it;
        }
    }
}

void AudioSystem::setListenerPosition(const Vec3& position) {
    m_listenerPosition = position;
    m_hasListenerPosition = true;
}

void AudioSystem::clearListenerPosition() {
    m_hasListenerPosition = false;
}

bool AudioSystem::isAvailable() const {
    return m_soloud != nullptr;
}

AudioSystem* AudioSystem::getActive() {
    return g_activeAudioSystem;
}