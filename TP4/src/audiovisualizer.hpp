#pragma once

#include <SFML/Audio.hpp>

#include <vector>
#include <cstdlib>
#include <cstdint>
#include <ctime>
#include <algorithm>

class AudioVisualizer {
public:
    AudioVisualizer();
    ~AudioVisualizer();

    bool loadMusic(const std::string& filename);
    void togglePlayback();
    void stopMusic();

    void update(float deltaTime);

    float getVolume() const { return currentVolume_; }

    bool isMusicLoaded() const { return soundBuffer_ != nullptr; }
    bool isMusicPlaying() const { return sound_->getStatus() == sf::Sound::Status::Playing; }

private:
    std::unique_ptr<sf::SoundBuffer> soundBuffer_;
    std::unique_ptr<sf::Sound> sound_;

    float currentVolume_ = 0.1f;
    float smoothedVolume_ = 0.1f;

    float thunderTimer_ = 0.0f;
    float nextThunderTime_ = 0.0f;

    std::size_t lastSampleIndex_ = 0;

    void analyzeRealAudio();
    void simulateThunder(float deltaTime);
    float calculateCurrentVolume();
};