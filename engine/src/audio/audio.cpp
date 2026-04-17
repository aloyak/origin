#include "engine/audio/audio.h"

#include "engine/utils/path.h"

#include <soloud.h>
#include <soloud_wav.h>

#include <spdlog/spdlog.h>

#include <algorithm>

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

    const float clampedVolume = clampVolume(volume);
    const SoLoud::handle handle = m_soloud->play(*cached->second, clampedVolume, 0.0f, false, 0);
    if (handle == 0) {
        return 0;
    }

    m_soloud->setLooping(handle, loop ? 1 : 0);
    return static_cast<SoundHandle>(handle);
}

void AudioSystem::stopSound(SoundHandle handle) {
    if (m_soloud == nullptr || handle == 0) {
        return;
    }

    m_soloud->stop(static_cast<SoLoud::handle>(handle));
}

void AudioSystem::stopAllSounds() {
    if (m_soloud != nullptr) {
        m_soloud->stopAll();
    }
}

void AudioSystem::setSoundVolume(SoundHandle handle, float volume) {
    if (m_soloud == nullptr || handle == 0) {
        return;
    }

    m_soloud->setVolume(static_cast<SoLoud::handle>(handle), clampVolume(volume));
}

void AudioSystem::setGlobalVolume(float volume) {
    if (m_soloud == nullptr) {
        return;
    }

    m_soloud->setGlobalVolume(clampVolume(volume));
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

