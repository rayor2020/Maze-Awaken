#pragma once

#include "../utils/maze_utils.h"
#include "../utils/position.h"
#include "game_level.h"
#include "game_object.h"

#include "../gaming/audio_manager.h"

#include <chrono>
#include <easyx.h>
#include <locale>
#include <map>
#include <memory>
#include <vector>
#define KEY_DOWN(hWnd, K) ((GetAsyncKeyState(K) & 0x8000 ? 1 : 0) && GetForegroundWindow() == hWnd)
// 来自 https://codebus.cn/yangw/transparent-putimage
// 处理半透明贴图
static void transparent_image(IMAGE* dstimg, int x, int y, IMAGE* srcimg)
{

    // 变量初始化
    DWORD* dst = GetImageBuffer(dstimg);
    DWORD* src = GetImageBuffer(srcimg);
    int src_width = srcimg->getwidth();
    int src_height = srcimg->getheight();
    int dst_width = (dstimg == NULL ? getwidth() : dstimg->getwidth());
    int dst_height = (dstimg == NULL ? getheight() : dstimg->getheight());
    int iwidth = (x + src_width > dst_width) ? dst_width - x : src_width;
    int iheight = (y + src_height > dst_height) ? dst_height - y : src_height;
    if (x < 0) {
        src += -x;
        iwidth -= -x;
        x = 0;
    }
    if (y < 0) {
        src += src_width * -y;
        iheight -= -y;
        y = 0;
    }
    dst += dst_width * y + x;
    for (int iy = 0; iy < iheight; ++iy) {
        for (int i = 0; i < iwidth; ++i) {
            int sa = (src[i] & 0xff000000) >> 24;
            if (sa != 0) {
                if (sa == 255)
                    dst[i] = src[i];
                else {
                    dst[i] = ((((src[i] & 0xff0000) >> 16) + ((dst[i] & 0xff0000) >> 16) * (255 - sa) / 255) << 16) | ((((src[i] & 0xff00) >> 8) + ((dst[i] & 0xff00) >> 8) * (255 - sa) / 255) << 8) | ((src[i] & 0xff) + (dst[i] & 0xff) * (255 - sa) / 255);
                }
            }
        }
        dst += dst_width;
        src += src_width;
    }
}

using obj_ptr = std::shared_ptr<game_object>;
using level_ptr = std::shared_ptr<game_level>;
class GameMain : public std::enable_shared_from_this<GameMain> {
private:
    std::vector<level_ptr> level_list;
    std::shared_ptr<player> now_player;
    level_ptr active_level; // 活跃关卡指针
    float_pos camera_pos;
    double current_fps;
    HWND hwnd;
    std::map<std::wstring, level_ptr> level_map;
    bool level_change_flag;
    level_ptr change_to_level;
    float_pos next_pos;
    int shift_animation_timer = 0;
    std::chrono::steady_clock::time_point start_time;
    IMAGE key_instruction;
    IMAGE collection[2];
    IMAGE message;
    bool creditting = false;
    int message_timer;
    bool cheated = false;
    bool triggered_message7 = false;
    bool triggered_message10 = false;
    bool has_collection_ironman = false;
    bool has_collection_weapon = false;
    std::queue<int> message_list;
    std::vector<int> key_input_list;
    const std::vector<int> mysterious_list = {
        VK_UP, VK_UP, VK_DOWN, VK_DOWN,
        VK_LEFT, VK_LEFT, VK_RIGHT, VK_RIGHT,
        'B', 'A', 'B', 'A'
    };

public:
    bool running;

private:
    bool in_camera(const obj_ptr obj) const
    {
        auto in_bound = [](const float_pos& p, const float_pos& p1, const float_pos& p2) {
            return p1.x <= p.x && p.x <= p2.x && p1.y <= p.y && p.y <= p2.y;
        };
        float_pos camera_up_left = { camera_pos.x - GAME_WINDOW_WIDTH * 0.5, camera_pos.y - GAME_WINDOW_HEIGHT * 0.5 },
                  camera_down_right = { camera_pos.x + GAME_WINDOW_WIDTH * 0.5, camera_pos.y + GAME_WINDOW_HEIGHT * 0.5 };
        float_pos img_range_1 = { obj->get_pos().x, obj->get_pos().y - obj->get_hitbox().y },
                  img_range_2 = { obj->get_pos().x + obj->get_hitbox().x, obj->get_pos().y - obj->get_hitbox().y },
                  img_range_3 = { obj->get_pos().x, obj->get_pos().y },
                  img_range_4 = { obj->get_pos().x + obj->get_hitbox().x, obj->get_pos().y };
        return (
            in_bound(img_range_1, camera_up_left, camera_down_right) || in_bound(img_range_2, camera_up_left, camera_down_right) || in_bound(img_range_3, camera_up_left, camera_down_right) || in_bound(img_range_4, camera_up_left, camera_down_right));
    }
    static bool on_press(int key)
    {
        static bool last_key_state[256] = { false };
        bool down = GetAsyncKeyState(key) & 0x8000, press_trigger = false;
        if (down && !last_key_state[key])
            press_trigger = true;
        last_key_state[key] = down;
        return press_trigger;
    }

public:
    GameMain(int width = GAME_WINDOW_WIDTH, int height = GAME_WINDOW_HEIGHT)
        : active_level(nullptr)
        , camera_pos(float_pos(GAME_WINDOW_WIDTH * 0.5, GAME_WINDOW_HEIGHT * 0.5))
        , current_fps(GAME_FPS)
        , change_to_level(nullptr)
        , running(true)
        , level_change_flag(false)
        , next_pos(float_pos())
        , has_collection_ironman(false)
        , has_collection_weapon(false)
    {
        initgraph(width, height);
        loadimage(&key_instruction, _T("assets/textures/gui/key_instruction.png"), 0, 0, false);
        loadimage(&collection[0], _T("assets/textures/item/ironman.png"), 0, 0, false);
        loadimage(&collection[1], _T("assets/textures/item/weapon.png"), 0, 0, false);
        loadimage(&message, _T("assets/textures/gui/messagebox.png"), 0, 0, false);
        setbkmode(TRANSPARENT);
        hwnd = GetHWnd();
        message_timer = -1;
        // setaspectratio(float(1.0f * GetSystemMetrics(SM_CXFULLSCREEN) / GAME_WINDOW_WIDTH),
        //    float(1.0f * GetSystemMetrics(SM_CYFULLSCREEN) / GAME_WINDOW_HEIGHT));
        cleardevice();
    }
    void send_message(int id)
    {
        message_list.push(id);
    }
    void register_level(const std::wstring& level_name, level_ptr level)
    {
        level_map.insert({ level_name, level });
    }
    level_ptr get_level_from_name(const std::wstring& level_name) const
    {
        auto iter = level_map.find(level_name);
        if (iter == level_map.end())
            return nullptr;
        else
            return iter->second;
    }
    void change_level(level_ptr next_level, const float_pos& new_pos)
    {
        level_change_flag = true;
        change_to_level = next_level;
        next_pos = new_pos;
        now_player->init();

        if (next_level->name() == L"maze_level_1B" || next_level->name() == L"assets\\levels\\level_1B.txt") {
            send_message(0);
            send_message(1);
        } else if (next_level->name() == L"maze_level_1C" || next_level->name() == L"assets\\levels\\level_1C.txt") {
            send_message(2);
            send_message(3);
        } else if (next_level->name() == L"maze_level_4D" || next_level->name() == L"assets\\levels\\level_4D.txt") {
            send_message(4);
        } else if (next_level->name() == L"maze_level_6C" || next_level->name() == L"assets\\levels\\level_6C.txt") {
            send_message(5);
        } else if (next_level->name() == L"maze_level_7B" || next_level->name() == L"assets\\levels\\level_7B.txt") {
            send_message(6);
        } else if (next_level->name() == L"maze_level_end" || next_level->name() == L"assets\\levels\\level_end.txt") {
            if (now_player->append_data() & 1)
                send_message(9);
            else
                send_message(8);
        } else
            ;
        if (now_player->death_counter >= 50 && !triggered_message7) {
            send_message(7);
            triggered_message7 = true;
        }
        shift_animation();
    }
    void restart()
    {
        level_change_flag = true;
        change_to_level = std::shared_ptr<game_level>(level_list[0]);
        next_pos = change_to_level->spawn_point();
        shift_animation();
        now_player->init(true);
        has_collection_ironman = false;
        has_collection_weapon = false;
        message_list = {};
        init();
    }
    void load_level()
    {
        for (std::size_t i = 0; i < LEVEL_TOTAL; ++i) {
            std::wstring file_name = level_id[i];
            std::wstring bg_path = L"assets\\textures\\background\\" + file_name + L".png";
            std::wstring dat_path = L"assets\\levels\\" + file_name + L".txt";
            level_ptr new_level = std::make_shared<game_level>(
                bg_path,
                dat_path,
                std::shared_ptr<GameMain>(this));
            level_list.push_back(new_level);
            level_map.insert({ file_name, new_level });
        }
    }
    void init()
    {
        active_level->set_player(now_player);
        active_level->start();
        now_player->set_pos(active_level->spawn_point());

        now_player->set_is_ironman(false);

        running = true;
        start_time = std::chrono::steady_clock::now();
    }
    void start()
    {
        load_level();
        active_level = level_list[0]; // 在此处修改开局关卡
        now_player = std::make_shared<player>(active_level);
        now_player->level_entered.insert(std::pair<std::wstring, bool>(active_level->name(), true));
        init();
        send_message(0);
        send_message(1);
    }
    void end()
    {
        closegraph();
    }
    void update_level()
    {
        if (active_level != nullptr && active_level->active()) {
            active_level->update();
        }
    }
    void shift_animation(bool to_credit = false)
    {
        shift_animation_timer = 90;
        creditting = to_credit;
    }
    void print_time() const
    {
        settextstyle(60, 0, _T("Consolas"));
        auto set_least_length = [](const std::wstring& str, int len) {
            if (str.length() > len)
                return str;
            return std::wstring(len - str.length(), L'0') + str;
        };
        auto now = std::chrono::steady_clock::now();
        long long millisecond = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count(),
                  second = millisecond / 1000,
                  minute = second / 60,
                  hour = minute / 60;
        std::wstring mls_string = set_least_length(std::to_wstring(millisecond % 1000), 3),
                     sec_string = set_least_length(std::to_wstring(second % 60), 2),
                     min_string = set_least_length(std::to_wstring(minute % 60), 2),
                     hour_string = set_least_length(std::to_wstring(hour), 2),
                     time_string = hour_string + L":" + min_string + L":" + sec_string + L":" + mls_string;
        settextcolor(BLACK);
        outtextxy(62, 62, time_string.c_str());
        settextcolor(WHITE);
        outtextxy(60, 60, time_string.c_str());
    }
    void print_info() const
    {
        settextstyle(40, 0, _T("Consolas"));
        settextcolor(WHITE);
        outtextxy(60, 140, (L"Death: " + std::to_wstring(now_player->death_time())).c_str());
        auto boss = active_level->enemy();
        if (boss)
            outtextxy(60, 200, (L"Boss HP: " + std::to_wstring(boss->get_hp())).c_str());
    }
    void print_collection()
    {
        int dat = now_player->append_data();
        if (dat & 1) {
            has_collection_ironman = true;
            putimage(260, 140, &collection[0], SRCPAINT);
        }
        if (dat & 2) {
            has_collection_weapon = true;
            putimage(340, 140, &collection[1], SRCPAINT);
        }
    }
    void print_message()
    {
        if (!running)
            return;
        if (message_timer > 0) {
            message_timer--;
            if (!message_list.empty()) {
                int msg_id = message_list.front();
                if (message_timer > 270)
                    putimage(480, -120 + (300 - message_timer) * 4,
                        960, 120, &message, 0, message_list.front() * 120);
                else if (message_timer < 30)
                    putimage(480, -120 + message_timer * 4,
                        960, 120, &message, 0, message_list.front() * 120);
                else
                    putimage(480, 0,
                        960, 120, &message, 0, message_list.front() * 120);
                if (message_timer == 0)
                    message_list.pop();
            }

        } else {
            if (!message_list.empty()) {
                message_timer = 300;
            }
        }
    }
    void draw_frame()
    {
        system("cls");
        std::cout << "fps: " << fps() << std::endl;
        BeginBatchDraw();
        if (shift_animation_timer > 0)
            shift_animation_timer--;
        else {
            shift_animation_timer = 0;
            running = true;
        }
        active_level->draw_level_frame(camera_pos);
        // 速通计时器输出
        print_time();
        // 玩家与敌人信息输出
        print_info();
        // 键盘提示绘制
        putimage(GAME_WINDOW_WIDTH - 16 - 384, GAME_WINDOW_HEIGHT - 16 - 64, &key_instruction, SRCPAINT);
        // 获取物品绘制
        print_collection();
        // 绘制提示消息
        print_message();
        // FlushBatchDraw();
    }
    bool has_collection() const
    {
        return has_collection_ironman;
    }
    void set_current_fps(double fps)
    {
        current_fps = fps;
    }
    int fps() const
    {
        return int(current_fps);
    }
    void keyboard_input()
    {
        const std::vector<int> mysterious_check = { VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT, 'B', 'A' };
        const std::size_t mysterious_length = mysterious_list.size();
        for (auto& key : mysterious_check) {
            if (on_press(key)) {
                key_input_list.push_back(key);
            }
        }
        std::cout << key_input_list.size() << std::endl;
        while (key_input_list.size() > mysterious_list.size())
            key_input_list.erase(key_input_list.begin());
        bool trigger = true;
        if (key_input_list.size() == mysterious_length) {
            for (std::size_t i = 0; i < mysterious_length; ++i) {
                if (key_input_list[i] != mysterious_list[i]) {
                    trigger = false;
                    break;
                }
            }
        } else {
            trigger = false;
        }
        if (trigger && !triggered_message10) {
            triggered_message10 = true;
            cheated = true;
            now_player->invincible = true;
            AudioManager& audioManager = AudioManager::getInstance();
            audioManager.playSound("wow_meme");
            send_message(10);
        }
        bool is_dashing, moving_left, moving_right, moving_up, moving_down, jump;
        moving_left = KEY_DOWN(hwnd, VK_LEFT);
        moving_right = KEY_DOWN(hwnd, VK_RIGHT);
        jump = KEY_DOWN(hwnd, VK_SPACE);
        moving_up = KEY_DOWN(hwnd, VK_UP);
        moving_down = KEY_DOWN(hwnd, VK_DOWN);
        is_dashing = KEY_DOWN(hwnd, 'X');
        printf("%d %d %d %d %d %d\n", jump, moving_left, moving_right, moving_up, moving_down, is_dashing);
        now_player->set_direction(MOVE_DIRECTION::STOP);
        now_player->set_jumping(jump);
        if (!is_dashing) {
            if (moving_left && !moving_right)
                now_player->set_direction(MOVE_DIRECTION::LEFT);
            if (moving_right && !moving_left)
                now_player->set_direction(MOVE_DIRECTION::RIGHT);
        } else {
            if (!now_player->can_dash())
                return;
            if (!moving_left && !moving_right && !moving_up && !moving_down && jump)
                now_player->set_direction(MOVE_DIRECTION::JUMP);
            if (moving_left && moving_up && !moving_right && !moving_down)
                now_player->set_direction(MOVE_DIRECTION::DASH_UP_LEFT);
            if (moving_right && moving_up && !moving_left && !moving_down)
                now_player->set_direction(MOVE_DIRECTION::DASH_UP_RIGHT);
            if (moving_left && moving_down && !moving_up && !moving_right)
                now_player->set_direction(MOVE_DIRECTION::DASH_DOWN_LEFT);
            if (moving_right && moving_down && !moving_up && !moving_left)
                now_player->set_direction(MOVE_DIRECTION::DASH_DOWN_RIGHT);
            if (moving_left && !moving_right && !moving_up && !moving_down)
                now_player->set_direction(MOVE_DIRECTION::DASH_LEFT);
            if (moving_right && !moving_left && !moving_up && !moving_down)
                now_player->set_direction(MOVE_DIRECTION::DASH_RIGHT);
            if (moving_up && !moving_down && !moving_left && !moving_right)
                now_player->set_direction(MOVE_DIRECTION::DASH_UP);
            if (moving_down && !moving_up && !moving_left && !moving_right)
                now_player->set_direction(MOVE_DIRECTION::DASH_DOWN);
        }
    }
    void message_trigger()
    {
    }
    void main_loop()
    {
        keyboard_input();
        if (running)
            update_level();
        message_trigger();
        if (level_change_flag) {
            active_level->deactivate();
            active_level = change_to_level;
            active_level->set_player(now_player);
            now_player->set_level(active_level);
            active_level->start();
            now_player->set_pos(next_pos);
            next_pos = float_pos();
            if (!now_player->level_entered.count(active_level->name())) {
                if (active_level->is_maze()) {
                    AudioManager& audioManager = AudioManager::getInstance();
                    audioManager.playMusic("battle_start");
                }
                now_player->level_entered.insert(std::pair<std::wstring, bool>(active_level->name(), true));
            }
            level_change_flag = false;
            change_to_level = nullptr;
        }
    }
    friend class GameWindow;
};
class GameWindow : public std::enable_shared_from_this<GameWindow> {
private:
    IMAGE assets[GAME_BACKGROUND_TOTAL];
    enum class ASSET_ID {
        titlescreen,
        pausescreen,
        title,
        shiftscreen,
        credit,
    };

    int current_animation_frame;
    int animation_frame_total;
    int frame_timer;
    int last_frame;
    GAME_STATUS status;
    GAME_STATUS last_status;
    std::shared_ptr<GameMain> game;

    HWND hwnd;
    bool go = false;
    bool esc = false;
    int credit_timer = 3120;

public:
    GameWindow()
    {
        loadimage(&assets[static_cast<int>(ASSET_ID::titlescreen)], _T("assets/textures/background/titlescreen.png"), GAME_WINDOW_WIDTH, GAME_WINDOW_HEIGHT * 64, true);
        loadimage(&assets[static_cast<int>(ASSET_ID::pausescreen)], _T("assets/textures/gui/paused.png"), GAME_WINDOW_WIDTH, GAME_WINDOW_HEIGHT, true);
        loadimage(&assets[static_cast<int>(ASSET_ID::title)], _T("assets/textures/gui/title.png"), 0, 0, false);
        loadimage(&assets[static_cast<int>(ASSET_ID::shiftscreen)], _T("assets/textures/background/room_background.png"), 1920, 1080, true);
        loadimage(&assets[static_cast<int>(ASSET_ID::credit)], _T("assets/textures/background/credit.png"), 0, 0, false);

        current_animation_frame = 0;
        animation_frame_total = 64;
        frame_timer = 0;
        status = last_status = GAME_STATUS::TITLESCREEN;
        game = nullptr;
        last_frame = -1;
        hwnd = GetHWnd();
    }
    void set_game(game_ptr new_game)
    {
        game = new_game;
    }
    void set_status(GAME_STATUS new_stat)
    {
        status = new_stat;
    }
    void keyboard_input()
    {
        if (KEY_DOWN(hwnd, '\r')) {
            if (!go && (status == GAME_STATUS::TITLESCREEN || status == GAME_STATUS::PAUSE)) {
                go = true;
                status = GAME_STATUS::GAMING;
            } else if (status == GAME_STATUS::CREDIT) {
                status = GAME_STATUS::END;
            }
        } else
            go = false;
        if (KEY_DOWN(hwnd, VK_ESCAPE)) {
            if (!esc /*&& (status == GAME_STATUS::PAUSE || status == GAME_STATUS::GAMING)*/) {
                esc = true;
                if (status == GAME_STATUS::TITLESCREEN)
                    status = GAME_STATUS::END;
                else if (status == GAME_STATUS::GAMING)
                    status = GAME_STATUS::PAUSE;
                else if (status == GAME_STATUS::PAUSE)
                    status = GAME_STATUS::END;
                else if (status == GAME_STATUS::CREDIT)
                    status = GAME_STATUS::END;
                else
                    ;
            }
        } else
            esc = false;
        if (KEY_DOWN(hwnd, 'R') && (status == GAME_STATUS::PAUSE || status == GAME_STATUS::CREDIT)) {
            status = GAME_STATUS::RESTART;
        }
    }
    void main_loop()
    {
        // cleardevice();
        keyboard_input();
        BeginBatchDraw();
        bool shift_flag = last_status != status;
        auto tmp_status = last_status;
        last_status = status;
        frame_timer++;
        AudioManager& audioManager = AudioManager::getInstance();
        if (game->shift_animation_timer > 0)
            status = GAME_STATUS::WAITING;
        else if (status == GAME_STATUS::WAITING) {
            if (game->creditting)
                status = GAME_STATUS::CREDIT;
            else
                status = GAME_STATUS::GAMING;
        } else
            ;
        switch (status) {
        case GAME_STATUS::TITLESCREEN:
            if (shift_flag)
                frame_timer = 0;
            last_frame = current_animation_frame;
            current_animation_frame = (frame_timer / 6) % animation_frame_total;
            if (current_animation_frame != last_frame) {
                putimage(0, 0, GAME_WINDOW_WIDTH, GAME_WINDOW_HEIGHT, &assets[static_cast<int>(ASSET_ID::titlescreen)],
                    0, GAME_WINDOW_HEIGHT * current_animation_frame);
                putimage((GAME_WINDOW_WIDTH - assets[static_cast<int>(ASSET_ID::title)].getwidth()) / 2, 240,
                    &assets[static_cast<int>(ASSET_ID::title)], SRCPAINT);
                last_frame = current_animation_frame;
            }
            break;
        case GAME_STATUS::PAUSE:
            game->running = false;
            // cleardevice();
            if (shift_flag) {
                audioManager.pauseMusic("bgm_normal");
                audioManager.playSound("gui_click");
                game->draw_frame();
                transparent_image(NULL, 0, 0, &assets[static_cast<int>(ASSET_ID::pausescreen)]);
            }
            break;
        case GAME_STATUS::GAMING:
            if (shift_flag && tmp_status != GAME_STATUS::WAITING) {
                audioManager.unpauseMusic("bgm_normal");
                audioManager.playSound("gui_click");
            }
            game->running = true;
            game->main_loop();
            game->draw_frame();
            break;
        case GAME_STATUS::WAITING:
            game->running = false;
            BeginBatchDraw();
            game->draw_frame();
            putimage(0,
                int(min(int(sqrt(45 * 45 - pow(game->shift_animation_timer - 45, 2)) * 40),
                        GAME_WINDOW_HEIGHT)
                    - assets[static_cast<int>(ASSET_ID::shiftscreen)].getheight()),
                &assets[static_cast<int>(ASSET_ID::shiftscreen)]);
            EndBatchDraw();
            break;
        case GAME_STATUS::CREDIT:
            if (shift_flag) {
                audioManager.stopAllMusic();
                audioManager.playMusic("credit", false);
            }
            if (credit_timer > 0) {
                credit_timer--;
                if (credit_timer < 3000)
                    putimage(0, -min(3096, (3000 - credit_timer) * 3096 / 2400), &assets[static_cast<int>(ASSET_ID::credit)]);
                else
                    putimage(0, 0, &assets[static_cast<int>(ASSET_ID::credit)]);
            } else
                ;
            break;
        case GAME_STATUS::END:
            game->end();
            exit(0);
            break;
        case GAME_STATUS::RESTART:
            game->restart();
            // status = GAME_STATUS::GAMING;
            audioManager.unpauseMusic("bgm_normal");
            break;
        }
        EndBatchDraw();
    }
};