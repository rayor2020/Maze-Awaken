#include "../header/gaming/audio_manager.h"
#include <filesystem>
#include <iostream>

AudioManager& AudioManager::getInstance()
{
    static AudioManager instance;
    return instance;
}

bool AudioManager::loadAudioFiles()
{
    // 获取当前工作目录
    std::string basePath = "./asset/sound/";

    try {
        // 加载音乐文件 (ogg格式)
        m_musicMap["bgm_ironman"] = new sf::Music();
        if (!m_musicMap["bgm_ironman"]->openFromFile(basePath + "bgm_ironman.ogg")) {
            std::cerr << "Failed to load bgm_ironman.ogg" << std::endl;
            return false;
        }

        m_musicMap["bgm_normal"] = new sf::Music();
        if (!m_musicMap["bgm_normal"]->openFromFile(basePath + "bgm_normal.ogg")) {
            std::cerr << "Failed to load bgm_normal.ogg" << std::endl;
            return false;
        }

        m_musicMap["battle_start"] = new sf::Music();
        if (!m_musicMap["battle_start"]->openFromFile(basePath + "battle_start.ogg")) {
            std::cerr << "Failed to load battle_start.ogg" << std::endl;
            return false;
        }

        m_musicMap["victory"] = new sf::Music();
        if (!m_musicMap["victory"]->openFromFile(basePath + "victory.ogg")) {
            std::cerr << "Failed to load victory.ogg" << std::endl;
            return false;
        }

        m_musicMap["credit"] = new sf::Music();
        if (!m_musicMap["credit"]->openFromFile(basePath + "credit.mp3")) {
            std::cerr << "Failed to load credit.mp3" << std::endl;
            return false;
        }
        // 加载音效文件
        sf::SoundBuffer* buffer;

        buffer = new sf::SoundBuffer();
        if (!buffer->loadFromFile(basePath + "gui_click.ogg")) {
            std::cerr << "Failed to load gui_click.ogg" << std::endl;
            return false;
        }
        m_soundBufferMap["gui_click"] = buffer;

        buffer = new sf::SoundBuffer();
        if (!buffer->loadFromFile(basePath + "player_jump0.ogg")) {
            std::cerr << "Failed to load player_jump0.ogg" << std::endl;
            return false;
        }
        m_soundBufferMap["player_jump0"] = buffer;

        buffer = new sf::SoundBuffer();
        if (!buffer->loadFromFile(basePath + "player_jump1.ogg")) {
            std::cerr << "Failed to load player_jump1.ogg" << std::endl;
            return false;
        }
        m_soundBufferMap["player_jump1"] = buffer;

        buffer = new sf::SoundBuffer();
        if (!buffer->loadFromFile(basePath + "player_jump2.ogg")) {
            std::cerr << "Failed to load player_jump2.ogg" << std::endl;
            return false;
        }
        m_soundBufferMap["player_jump2"] = buffer;

        buffer = new sf::SoundBuffer();
        if (!buffer->loadFromFile(basePath + "player_death0.ogg")) {
            std::cerr << "Failed to load player_death0.ogg" << std::endl;
            return false;
        }
        m_soundBufferMap["player_death0"] = buffer;

        buffer = new sf::SoundBuffer();
        if (!buffer->loadFromFile(basePath + "player_death1.ogg")) {
            std::cerr << "Failed to load player_death1.ogg" << std::endl;
            return false;
        }
        m_soundBufferMap["player_death1"] = buffer;

        buffer = new sf::SoundBuffer();
        if (!buffer->loadFromFile(basePath + "player_death2.ogg")) {
            std::cerr << "Failed to load player_death2.ogg" << std::endl;
            return false;
        }
        m_soundBufferMap["player_death2"] = buffer;

        buffer = new sf::SoundBuffer();
        if (!buffer->loadFromFile(basePath + "player_death3.ogg")) {
            std::cerr << "Failed to load player_death3.ogg" << std::endl;
            return false;
        }
        m_soundBufferMap["player_death3"] = buffer;
        buffer = new sf::SoundBuffer();
        if (!buffer->loadFromFile(basePath + "bullet.ogg")) {
            std::cerr << "Failed to load bullet.ogg" << std::endl;
            return false;
        }
        m_soundBufferMap["bullet"] = buffer;
        buffer = new sf::SoundBuffer();
        if (!buffer->loadFromFile(basePath + "laser_powering.ogg")) {
            std::cerr << "Failed to load laser_powering.ogg" << std::endl;
            return false;
        }
        m_soundBufferMap["laser_powering"] = buffer;
        buffer = new sf::SoundBuffer();
        if (!buffer->loadFromFile(basePath + "laser_fire.mp3")) {
            std::cerr << "Failed to load laser_fire.mp3" << std::endl;
            return false;
        }
        m_soundBufferMap["laser_fire"] = buffer;
        buffer = new sf::SoundBuffer();
        if (!buffer->loadFromFile(basePath + "boss_hit.mp3")) {
            std::cerr << "Failed to load boss_hit.mp3" << std::endl;
            return false;
        }
        m_soundBufferMap["boss_hit"] = buffer;
        buffer = new sf::SoundBuffer();
        if (!buffer->loadFromFile(basePath + "iron_man_fail.mp3")) {
            std::cerr << "Failed to load iron_man_fail.mp3" << std::endl;
            return false;
        }
        m_soundBufferMap["iron_man_fail"] = buffer;
        buffer = new sf::SoundBuffer();
        if (!buffer->loadFromFile(basePath + "wow_meme.mp3")) {
            std::cerr << "Failed to load wow_meme.mp3" << std::endl;
            return false;
        }
        m_soundBufferMap["wow_meme"] = buffer;
        // 初始化声音对象
        for (const auto& pair : m_soundBufferMap) {
            sf::Sound* sound = new sf::Sound(*pair.second);
            m_soundMap[pair.first] = sound;
        }
        
        std::cout << "Successfully loaded all audio files!" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading audio files: " << e.what() << std::endl;
        return false;
    }
}

void AudioManager::playMusic(const std::string& name, bool loop)
{
    auto it = m_musicMap.find(name);
    if (it != m_musicMap.end()) {
        it->second->setLooping(loop);
        it->second->setVolume(m_musicVolume);
        it->second->play();
    }
}

void AudioManager::playSound(const std::string& name)
{
    auto it = m_soundMap.find(name);
    if (it != m_soundMap.end()) {
        it->second->setVolume(m_soundVolume);
        it->second->play();
    }
}

void AudioManager::stopMusic(const std::string& name)
{
    auto it = m_musicMap.find(name);
    if (it != m_musicMap.end()) {
        it->second->stop();
    }
}
void AudioManager::pauseMusic(const std::string& name){
    auto it = m_musicMap.find(name);
    if (it != m_musicMap.end()) {
        it->second->pause();
    }
}
void AudioManager::pauseAllMusic(){
    for (auto& pair : m_musicMap) {
        pair.second->pause();
    }
}
void AudioManager::unpauseMusic(const std::string& name) {
    auto it = m_musicMap.find(name);
    if (it != m_musicMap.end()) {
        it->second->play();
    }
}
void AudioManager::unpauseAllMusic() {
    for (auto& pair : m_musicMap) {
        pair.second->play();
    }
}
void AudioManager::stopAllMusic()
{
    for (auto& pair : m_musicMap) {
        pair.second->stop();
    }
}

void AudioManager::setMusicVolume(float volume)
{
    m_musicVolume = volume;
    for (auto& pair : m_musicMap) {
        pair.second->setVolume(volume);
    }
}

void AudioManager::setSoundVolume(float volume)
{
    m_soundVolume = volume;
    for (auto& pair : m_soundMap) {
        pair.second->setVolume(volume);
    }
}

AudioManager::~AudioManager()
{
    // 清理音乐资源
    for (auto& pair : m_musicMap) {
        delete pair.second;
    }
    m_musicMap.clear();

    // 清理声音缓冲区资源
    for (auto& pair : m_soundBufferMap) {
        delete pair.second;
    }
    m_soundBufferMap.clear();

    // 清理声音资源
    for (auto& pair : m_soundMap) {
        delete pair.second;
    }
    m_soundMap.clear();
}