#pragma once

#include <SFML/Audio.hpp>
#include <map>
#include <string>

class AudioManager {
public:
    static AudioManager& getInstance();

    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    bool loadAudioFiles();
    void playMusic(const std::string& name, bool loop = false);
    void playSound(const std::string& name);
    void stopMusic(const std::string& name);
    void pauseAllMusic();
    void pauseMusic(const std::string& name);
    void stopAllMusic();
    void unpauseMusic(const std::string& name);
    void unpauseAllMusic();
    void setMusicVolume(float volume);
    void setSoundVolume(float volume);

private:
    AudioManager() = default;
    ~AudioManager();

    std::map<std::string, sf::Music*> m_musicMap;
    std::map<std::string, sf::SoundBuffer*> m_soundBufferMap;
    std::map<std::string, sf::Sound*> m_soundMap;

    float m_musicVolume = 100.0f;
    float m_soundVolume = 100.0f;
};