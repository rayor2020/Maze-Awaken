#include "../header/gaming/game_level.h"
#include "../header/gaming/game_main.h"
#include "../header/utils/maze_utils.h"
#include "../header/utils/position.h"
#include <easyx.h>
#include <fstream>
#include <iostream>
std::vector<std::wstring> game_level::split(const std::wstring& line, wchar_t space)
{
    std::size_t s = line.length();
    std::vector<std::wstring> res;
    for (std::size_t i = 0; i < s; ++i) {
        if (line[i] != space) {
            std::wstring word;
            while (line[i] != space && i < s) {
                word += line[i];
                i++;
            }
            res.emplace_back(word);
        }
    }
    return res;
}

// 坐标转换
float_pos game_level::to_screen_coord(const float_pos& real_coord, const float_pos& camera_pos) const
{
    return { real_coord.x - camera_pos.x + screen_center_x,
        real_coord.y - camera_pos.y + screen_center_y };
}

void game_level::load_level_from(const std::wstring& level_dat)
{
    std::wifstream level_file;
    level_file.open(level_dat.c_str(), std::wios::in);
    if (!level_file.is_open()) {
        std::wcout << L"error opening " << level_dat << std::endl;
        return;
    }
    std::wstring name;
    level_file >> name;
    level_name = name.substr(1, name.length() - 1);
    in_game->register_level(level_name, shared_from_this());
    std::wstring obj_line;
    std::vector<std::wstring> file_lines;
    while (std::getline(level_file, obj_line)) {
        file_lines.push_back(obj_line);
    }
    for (auto& line : file_lines) {
        auto words = split(line);
        if (words.empty())
            continue;
        if (words[0] == L"spawn:") {
            int x = std::stoi(words[1]),
                y = std::stoi(words[2]);
            player_spawn_point = float_pos(x, y);
        } else if (words[0] == L"platform:") {
            // arg1: x, arg2: y, arg3: width, arg4: height, arg5: is_dangerous
            int x = std::stoi(words[1]),
                y = std::stoi(words[2]),
                width = std::stoi(words[3]),
                height = std::stoi(words[4]),
                is_dangerous = std::stoi(words[5]);
            add_object(std::make_shared<platform>(
                shared_from_this(),
                float_pos(double(x), double(y)),
                width, height, bool(is_dangerous)));
        } else if (words[0] == L"spike:") {
            // arg1: x, arg2: y, arg3: width, arg4: height
            int x = std::stoi(words[1]),
                y = std::stoi(words[2]),
                width = std::stoi(words[3]),
                height = std::stoi(words[4]);
            MOVE_DIRECTION direction = (words[5] == L"up" ? MOVE_DIRECTION::UP : (words[5] == L"right" ? MOVE_DIRECTION::RIGHT : (words[5] == L"down" ? MOVE_DIRECTION::DOWN : MOVE_DIRECTION::LEFT)));
            add_object(std::make_shared<spike>(
                shared_from_this(),
                float_pos(double(x), double(y)),
                width, height, direction));
        } else if (words[0] == L"trigger:") {
            // arg1: x, arg2: y, arg3: width, arg4: height, arg5: destination, arg6: type
            // arg7: land at x arg8: land at y
            int x = std::stoi(words[1]),
                y = std::stoi(words[2]),
                width = std::stoi(words[3]),
                height = std::stoi(words[4]);
            std::wstring dst = words[5];
            if (dst == L"end") {
                add_object(std::make_shared<level_trigger>(
                    float_pos(x, y), float_pos(width, height), shared_from_this(), dst,
                    TRIGGER_TYPE::END,
                    float_pos(0, 0)));
            } else {
                int land_x = std::stoi(words[7]),
                    land_y = std::stoi(words[8]);
                add_object(std::make_shared<level_trigger>(
                    float_pos(x, y), float_pos(width, height), shared_from_this(), dst,
                    words[6] == L"bound" ? TRIGGER_TYPE::BOUND : (words[6] == L"speed" ? TRIGGER_TYPE::SPEED : TRIGGER_TYPE::SPEED),
                    float_pos(land_x, land_y)));
            }
        } else if (words[0] == L"maze:") {
            is_maze_level = (words[1][0] - L'0');
        } else if (words[0] == L"binary:") {
            router = maze_router(words[1]);
            std::size_t len = words[1].length();
            for (std::size_t index = 0; index < len; ++index) {
                game_maze_map[index / MAZE_WIDTH][index % MAZE_WIDTH] = (words[1][index] - L'0');
            }
        } else if (words[0] == L"boss:") {
            int type = std::stoi(words[1]),
                x = std::stoi(words[2]),
                y = std::stoi(words[3]);
            switch (type) {
            case 1:
                add_object(std::make_shared<boss1>(shared_from_this(), float_pos(x, y)));
                break;
            case 2:
                add_object(std::make_shared<boss2>(shared_from_this(), float_pos(x, y)));
                break;
            case 3:
                add_object(std::make_shared<boss3>(shared_from_this(), float_pos(x, y)));
                break;
            default:
                break;
            }
        } else if (words[0] == L"gate:") {
            int x = std::stoi(words[1]),
                y = std::stoi(words[2]),
                width = std::stoi(words[3]),
                height = std::stoi(words[4]);
            add_object(std::make_shared<gate>(shared_from_this(), float_pos(x, y), width, height));
        } else if (words[0] == L"collection:") {
            if (words[1] == L"iron_man") {
                int x = std::stoi(words[2]),
                    y = std::stoi(words[3]);
                if (!game_instance()->has_collection())
                    add_object(std::make_shared<collection_ironman>(float_pos(x, y), shared_from_this()));
            } else if (words[1] == L"weapon") {
                int x = std::stoi(words[2]),
                    y = std::stoi(words[3]);
                add_object(std::make_shared<collection_weapon>(float_pos(x, y), shared_from_this()));
            } else
                ;
        } else
            ;
    }
}
game_level::game_level(const std::wstring& background_path, const std::wstring& level_dat, game_ptr game)
{
    is_active = false;
    is_maze_level = false;
    for (int y = 0; y < GAME_WINDOW_HEIGHT / 60; ++y) {
        for (int x = 0; x < GAME_WINDOW_WIDTH / 60; ++x)
            game_maze_map[y][x] = false;
    }
    player_spawn_point = float_pos();
    in_game = game;
    loadimage(&background, background_path.c_str(), GAME_WINDOW_WIDTH, GAME_WINDOW_HEIGHT, true);
    level_name = level_dat;
    level_dat_path = level_dat;
    holding_player = nullptr;
    // 预计算屏幕中心偏移值
    screen_center_x = GAME_WINDOW_WIDTH * 0.5f;
    screen_center_y = GAME_WINDOW_HEIGHT * 0.5f;
}
std::shared_ptr<game_level> game_level::get_level_from_name(const std::wstring& name)
{
    return in_game->get_level_from_name(name);
}
void game_level::start()
{
    is_active = true;
    if (obj_list.empty())
        load_level_from(level_dat_path);
    // holding_player->respawn();
}
void game_level::reset()
{
    obj_list = {};
    load_level_from(level_dat_path);
}
bool game_level::active() const
{
    return is_active;
}
bool game_level::is_maze() const
{
    return is_maze_level;
}
std::shared_ptr<player> game_level::get_player()
{
    return holding_player;
}
void game_level::change_level(std::shared_ptr<game_level> next_level, const float_pos& new_pos)
{
    in_game->change_level(next_level, new_pos);
}
void game_level::restart_game()
{
    in_game->restart();
}
void game_level::shift_animation(bool to_credit)
{
    in_game->shift_animation(to_credit);
}
void game_level::deactivate()
{
    is_active = false;
    holding_player = nullptr;
    obj_list = {};
}
void game_level::add_object(obj_ptr new_obj)
{
    obj_list.push_back(new_obj);
}
void game_level::add_object_request(obj_ptr new_obj)
{
    new_obj_list.push_back(new_obj);
}
void game_level::draw_level_frame(const float_pos& camera_pos) const
{
    float_pos bg_pos = to_screen_coord({ 0.0, 0.0 }, camera_pos);
    putimage(int(bg_pos.x), int(bg_pos.y), &background);
    if (holding_player != nullptr)
        holding_player->draw_on(to_screen_coord(holding_player->get_pos(), camera_pos));
    for (auto& obj : obj_list) {
        obj->draw_on(to_screen_coord(obj->get_pos(), camera_pos));
    }
}
float_pos game_level::spawn_point() const
{
    return player_spawn_point;
}
std::wstring game_level::name() const
{
    return level_name;
}
// 帧更新
void game_level::update()
{
    if (holding_player != nullptr)
        holding_player->update();
    if (!obj_list.empty() && is_active) {
        for (auto& obj : obj_list)
            obj->update();
        obj_list.erase(std::remove_if(obj_list.begin(), obj_list.end(),
                           [](const obj_ptr& obj) {
                               return obj->get_hp() == 0 && obj->get_type() != OBJECT_TYPE::PLAYER;
                           }),
            obj_list.end());
        for (auto& obj : new_obj_list) {
            obj_list.push_back(obj);
        }
        new_obj_list = {};
    }
}
void game_level::set_player(std::shared_ptr<player> p)
{
    holding_player = p;
}
game_ptr game_level::game_instance()
{
    return in_game;
}
bool game_level::maze_map(const int_pos& p) const
{
    if (p.x < 0 || p.x >= MAZE_WIDTH || p.y < 0 || p.y >= MAZE_HEIGHT)
        return false;
    return game_maze_map[p.y][p.x];
}
bool game_level::hitbox_intersect(const float_pos& box1_ul, const float_pos& box1_dr, const float_pos& box2_ul, const float_pos& box2_dr)
{
    float_pos center1 = (box1_ul + box1_dr) * 0.5,
              center2 = (box2_ul + box2_dr) * 0.5;
    return (fabs(center2.x - center1.x) <= fabs(box1_dr.x - box1_ul.x + box2_dr.x - box2_ul.x) * 0.5 && fabs(center2.y - center1.y) <= fabs(box1_dr.y - box1_ul.y + box2_dr.y - box2_ul.y) * 0.5);
}
bool game_level::collide(const obj_ptr& obj1, const obj_ptr& obj2)
{
    float_pos obj1_pos = obj1->get_pos(), obj1_delta = obj1->get_hitbox_delta(),
              obj2_pos = obj2->get_pos(), obj2_delta = obj2->get_hitbox_delta();
    float_pos pos1(obj1_pos.x + obj1_delta.x, obj1_pos.y + obj1_delta.y), box1 = obj1->get_hitbox(),
                                                                          pos2(obj2_pos.x + obj2_delta.x, obj2_pos.y + obj2_delta.y), box2 = obj2->get_hitbox();
    float_pos box1_ul(pos1),
        box1_dr(pos1 + box1),
        box2_ul(pos2),
        box2_dr(pos2 + box2);
    // std::cout << pos1 << ", " << pos2 << ", " << box1_ul << ", " << box1_dr << ", " << box2_ul << ", " << box2_dr << std::endl;
    return hitbox_intersect(box1_ul, box1_dr, box2_ul, box2_dr);
}
std::vector<obj_ptr> game_level::collide_with(const obj_ptr& from_obj)
{
    std::vector<obj_ptr> collide_list;
    if (holding_player != nullptr && from_obj->get_type() != OBJECT_TYPE::PLAYER) {
        if (collide(from_obj, holding_player)) {
            collide_list.push_back(holding_player);
        }
    }
    if (obj_list.empty())
        return collide_list;
    for (auto& obj : obj_list) {
        if (obj->is_physic() && obj->get_id() != from_obj->get_id() && collide(from_obj, obj)) {
            collide_list.push_back(obj);
        }
    }

    return collide_list;
}
std::queue<int_pos> game_level::route(const int_pos& p1, const int_pos& p2)
{
    return router.point_of_route(p1, p2);
}
bool game_level::has(OBJECT_TYPE type)
{
    return std::find_if(obj_list.begin(), obj_list.end(), [&type](const obj_ptr& obj) {
        return obj->get_type() == type;
    }) != obj_list.end();
}
obj_ptr game_level::enemy()
{
    if (has(OBJECT_TYPE::ENEMY))
        return *std::find_if(obj_list.begin(), obj_list.end(), [](const obj_ptr& obj) {
            return obj->get_type() == OBJECT_TYPE::ENEMY;
        });
    else
        return nullptr;
}