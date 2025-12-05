#include <SFML/Audio.hpp>
#include "audiovisualizer.hpp"
#include <iostream>
#include <cmath>

AudioVisualizer::AudioVisualizer() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    nextThunderTime_ = 3.0f + (std::rand() % 100) * 0.05f;
}



AudioVisualizer::~AudioVisualizer() {
    if (sound_->getStatus() == sf::Sound::Status::Playing) {
        sound_->stop();
    }
}

bool AudioVisualizer::loadMusic(const std::string& filename) {
    soundBuffer_ = std::make_unique<sf::SoundBuffer>();

    if (!soundBuffer_->loadFromFile(filename)) {
        std::cerr << "Failed to load music: " << filename << std::endl;
        soundBuffer_.reset();
        sound_.reset();
        return false;
    }

    sound_ = std::make_unique<sf::Sound>(*soundBuffer_);

    std::cout << "Music loaded successfully: " << filename << std::endl;
    return true;
}



void AudioVisualizer::togglePlayback() {
    if (!isMusicLoaded()) return;

    if (isMusicPlaying()) {
        sound_->pause();
        currentVolume_ = 0.1f;
    }
    else {
        sound_->play();
        lastSampleIndex_ = 0;
    }
}

void AudioVisualizer::stopMusic() {
    if (isMusicLoaded()) {
        sound_->stop();
        currentVolume_ = 0.1f;
    }
}

void AudioVisualizer::update(float deltaTime) {
    if (isMusicLoaded() && isMusicPlaying()) {
        analyzeRealAudio();
    }
    else {
        simulateThunder(deltaTime);
    }
}

void AudioVisualizer::analyzeRealAudio() {
    float newVolume = calculateCurrentVolume();

    const float smoothing = 0.1f;
    smoothedVolume_ = smoothedVolume_ * (1.0f - smoothing) + newVolume * smoothing;

    currentVolume_ = 0.1f + smoothedVolume_ * 0.5f;
    currentVolume_ = std::max(0.1f, std::min(1.0f, currentVolume_));
}

float AudioVisualizer::calculateCurrentVolume() {
    if (!soundBuffer_) return 0.1f;
    
    sf::Time offset = sound_->getPlayingOffset();
    
    unsigned int sampleRate = soundBuffer_->getSampleRate();
    unsigned int channelCount = soundBuffer_->getChannelCount();
    std::size_t currentSample = static_cast<std::size_t>(
        offset.asSeconds() * sampleRate * channelCount
    );
    
    const int16_t* samples = soundBuffer_->getSamples();
    std::size_t sampleCount = soundBuffer_->getSampleCount();
    
    if (sampleCount == 0) return 0.1f;

    const std::size_t windowSize = 512;
    int peak = 0;
    std::size_t analyzed = 0;
    
    for (std::size_t i = 0; i < windowSize && currentSample + i < sampleCount; i++) {
        int sample = std::abs(samples[currentSample + i]);
        if (sample > peak) {
            peak = sample;
        }
        analyzed++;
    }
    
    if (analyzed == 0) return 0.1f;
    
    float normalized = static_cast<float>(peak) / 32767.0f;
    
    normalized = std::sqrt(normalized);
    return std::max(0.1f, std::min(1.0f, 0.1f + normalized * 0.9f));
}

void AudioVisualizer::simulateThunder(float deltaTime) {
    thunderTimer_ += deltaTime;

    if (thunderTimer_ >= nextThunderTime_) {
        currentVolume_ = 0.7f;
        thunderTimer_ = 0.0f;
        nextThunderTime_ = 5.0f + (std::rand() % 100) * 0.1f;
    }
    else if (thunderTimer_ < 0.2f) {
        currentVolume_ = 0.7f;
    }
    else if (thunderTimer_ < 0.4f) {
        float fade = (0.4f - thunderTimer_) / 0.2f;
        currentVolume_ = 0.7f * fade;
    }
    else {
        currentVolume_ = 0.1f;
    }
}