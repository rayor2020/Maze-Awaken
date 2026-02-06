#include "header/gaming/game_main.h"
#include <chrono>
#include <easyx.h>
#include <iostream>
#include <thread>

#include "header/gaming/audio_manager.h"
#include <SFML/Audio.hpp>
#include <windows.h>
#pragma comment(lib, "shcore.lib")
int main()
{
    typedef HRESULT(WINAPI * SetProcessDpiAwarenessFunc)(int);
    HMODULE hShCore = LoadLibraryW(L"Shcore.dll");
    if (hShCore) {
        SetProcessDpiAwarenessFunc pSetProcessDpiAwareness = (SetProcessDpiAwarenessFunc)GetProcAddress(hShCore, "SetProcessDpiAwareness");
        if (pSetProcessDpiAwareness) {
            pSetProcessDpiAwareness(1); // PROCESS_PER_MONITOR_DPI_AWARE
        }
        FreeLibrary(hShCore);
    }
    using clock = std::chrono::steady_clock;
    using frame_rate = std::chrono::duration<int, std::ratio<1, GAME_FPS>>;
    GameMain game;
    GameWindow window;
    window.set_game(std::shared_ptr<GameMain>(&game));
    AudioManager& audioManager = AudioManager::getInstance();
    if (!audioManager.loadAudioFiles()) {
        std::cerr << "Failed to load audio files!" << std::endl;
        return -1;
    }
    audioManager.playMusic("bgm_normal", true);
    sf::Music bgmNormal, bgmIronman, victory;
    sf::SoundBuffer engageBuffer, clickBuffer;
    sf::Sound engage(engageBuffer), click(clickBuffer);

    game.start();
    auto start_clock = clock().now();
    auto next = start_clock + frame_rate { 1 };
    auto now = clock().now();
    int frame_count = 0;
    while (true) {
        std::this_thread::sleep_until(next);
        next += frame_rate { 1 };
        window.main_loop();
        game.set_current_fps(int(1000.0 * frame_count / std::chrono::duration_cast<std::chrono::milliseconds>(clock().now() - start_clock).count()));
        frame_count++;
    }
    return 0;
}