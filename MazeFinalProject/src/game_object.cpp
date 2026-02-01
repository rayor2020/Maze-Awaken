#include "../header/gaming/game_object.h"
#include "../header/gaming/audio_manager.h"
#include "../header/gaming/game_level.h"
#include "../header/utils/maze_rand.h"
#include "../header/utils/maze_utils.h"
#include "../header/utils/position.h"
#include <algorithm>
#include <cmath>
#include <easyx.h>
#include <iostream>
#include <numbers>
#include <set>
#include <thread>
#pragma comment(lib, "MSIMG32.lib")
static void draw_image(IMAGE& image, int x, int y, int w = -1, int h = -1)
{
    BLENDFUNCTION blendfunc = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
    int width = image.getwidth();
    int height = image.getheight();
    if (w == -1)
        w = width;
    if (h == -1)
        h = height;
    AlphaBlend(GetImageHDC(), x, y, w, h, GetImageHDC(&image), 0, 0, width, height, blendfunc);
}
static rand_generator rgen;
game_object::game_object(
    const float_pos& pos,
    const float_pos& collide_box,
    bool collide,
    int hp,
    int power_point,
    int img_height,
    int img_width,
    const float_pos& velocity,
    bool is_static,
    int frame_interval)
    : at_level(nullptr)
    , pos(pos)
    , collide(true)
    , collide_box(collide_box)
    , collide_delta(float_pos())
    , hp(1)
    , power_point(1)
    , velocity(velocity)
    , is_static(is_static)
    , img_height(0)
    , img_width(0)
    , img(NULL)
    , total_animation_frame(1)
    , current_animation_frame(0)
    , frame_interval(frame_interval)
    , id(global_object_id++)
    , type(OBJECT_TYPE::NONE)
{
}
float_pos game_object::get_pos() const
{
    return pos;
}
void game_object::set_pos(double x, double y)
{
    pos = float_pos(x, y);
}
void game_object::set_pos(const float_pos& p)
{
    pos = p;
}
void game_object::set_hp(int new_hp)
{
    hp = new_hp;
}
void game_object::set_level(std::shared_ptr<game_level> new_level)
{
    at_level = new_level;
}
int game_object::get_hp() const
{
    return hp;
}
float_pos game_object::get_velocity() const
{
    return velocity;
}
OBJECT_TYPE game_object::get_type() const
{
    return type;
}
float_pos game_object::get_hitbox() const
{
    return collide_box;
}
float_pos game_object::get_center() const
{
    return pos + collide_delta + collide_box * 0.5;
}
float_pos game_object::get_hitbox_delta() const
{
    return collide_delta;
}
int game_object::get_id() const
{
    return id;
}
int game_object::get_power_point() const
{
    return power_point;
}
bool game_object::is_physic() const
{
    return collide;
}
void game_object::draw_on(const float_pos& screen_pos)
{
    int frame_now = frame_interval > 1 ? (current_animation_frame % frame_interval) : 0;
    int img_height = img.getheight(), img_width = img.getwidth();
    putimage(int(screen_pos.x), int(screen_pos.y),
        img_width, img_height,
        &img,
        frame_now * img_width, 0);
}
void game_object::attack(int damage)
{
    hp = max(0, hp - damage);
}
int game_object::append_data()
{
    return 0;
}

player::player(std::shared_ptr<game_level> level)
{
    at_level = level;
    is_static = false;
    death_counter = 0;
    pos = float_pos(0.0, 0.0);
    collide_box = float_pos(24.0, 32.0);
    collide_delta = float_pos(20.0, 32.0);
    load_image_from_path(&img, L"asset\\image\\player.png", 64, 64, true);
    img_height = 64;
    img_width = 64;
    current_animation_frame = 0;
    frame_interval = 3;
    type = OBJECT_TYPE::PLAYER;
    invincible = false;
    load_image_from_path(&animation_list[0], L"asset\\image\\player_stand.png");
    load_image_from_path(&animation_list[1], L"asset\\image\\player_stand_rev.png");
    load_image_from_path(&animation_list[2], L"asset\\image\\player_stand_mask.png");
    load_image_from_path(&animation_list[3], L"asset\\image\\player_stand_mask_rev.png");
    load_image_from_path(&animation_list[4], L"asset\\image\\player_walk.png");
    load_image_from_path(&animation_list[5], L"asset\\image\\player_walk_rev.png");
    load_image_from_path(&animation_list[6], L"asset\\image\\player_walk_mask.png");
    load_image_from_path(&animation_list[7], L"asset\\image\\player_walk_mask_rev.png");
    init();
}
void player::init(bool is_restart)
{
    hp = 1;
    if (is_restart) {
        power_point = 1;
        level_entered = {};
        is_ironman = false;
    }
    direction = MOVE_DIRECTION::STOP;
    velocity = float_pos();
    from_last_jump = 0;
    from_last_dash = 0;
    jump_timer = 0;
    input_timer = 0;
    dash_time = -1;
    dash_counter = 1;
    dash_direction = MOVE_DIRECTION::STOP;
    dash_velocity = float_pos();
    falling_timer = 0;
    // is_ironman = false;
    reverse = false;
    on_ground = false;
    jumping = false;
    dashing = false;
    walking = false;
    hyper_dashing = false;
    keeping_grounded = false;
}

void player::set_is_ironman(bool new_state)
{
    is_ironman = new_state;
}

void player::update()
{
    move();
    checking_alive();
}
void player::collision()
{
    auto collide_list = at_level->collide_with(shared_from_this());
    on_ground = false;
    for (auto& obj : collide_list) {
        if (obj->get_type() != OBJECT_TYPE::PLATFORM)
            continue;
        if (pos.y + collide_box.y <= obj->get_pos().y + 1) {
            on_ground = true;
            falling_timer = 0;
            break;
        }
    }
    // X轴移动
    std::set<int> hit_list;
    for (int i = 0; i < 10; ++i) {
        pos.x += velocity.x * 0.1;
        collide_list = at_level->collide_with(shared_from_this());
        for (auto& obj : collide_list) {
            if (obj->get_type() != OBJECT_TYPE::PLATFORM)
                continue;
            if (pos.y <= obj->get_pos().y - collide_box.y || pos.y >= obj->get_pos().y + obj->get_hitbox().y) {
                if (pos.y - 10 <= obj->get_pos().y - collide_box.y) {
                    pos.y = obj->get_pos().y - collide_box.y - collide_delta.y - 0.1;
                    velocity.y = 0.0;
                    on_ground = true;
                    if (obj->get_type() == OBJECT_TYPE::PLATFORM)
                        obj->trigger();
                    if (dash_time == -1)
                        dash_counter = 1;
                    falling_timer = 0;
                } else
                    continue;
            } else {
                if (velocity.x > 0.0) {
                    pos.x = obj->get_pos().x - collide_box.x - collide_delta.x - 0.1;
                    velocity.x = 0.0;
                    hit_list.insert(obj->get_id());
                } else if (velocity.x < 0.0) {
                    pos.x = obj->get_pos().x + obj->get_hitbox().x - collide_delta.x + 0.1;
                    velocity.x = 0.0;
                    hit_list.insert(obj->get_id());
                } else
                    ;
            }
        }
    }
    if (!dashing && !on_ground) {
        if (MAX_FALLING_SPEED - velocity.y > 0.1)
            velocity.y += 1.4;
        else
            velocity.y = MAX_FALLING_SPEED;
        falling_timer++;
    }
    for (int i = 0; i < 10; ++i) {
        pos.y += velocity.y * 0.1;
        collide_list = at_level->collide_with(shared_from_this());
        for (auto& obj : collide_list) {
            if (obj->get_type() != OBJECT_TYPE::PLATFORM)
                continue;
            if (hit_list.count(obj->get_id()))
                continue;
            if (pos.y >= obj->get_pos().y + obj->get_hitbox().y && pos.y <= obj->get_pos().y - collide_box.y)
                continue;
            if (velocity.y < 0.0) {
                pos.y = obj->get_pos().y + obj->get_hitbox().y - collide_delta.y + 0.1;
                velocity.y = 0.0;
            } else if (velocity.y > 0.0) {
                pos.y = obj->get_pos().y - collide_box.y - collide_delta.y - 0.1;
                velocity.y = 0.0;
                on_ground = true;
                if (obj->get_type() == OBJECT_TYPE::PLATFORM)
                    obj->trigger();
                if (dash_time == -1)
                    dash_counter = 1;
                falling_timer = 0;
            } else
                ;
        }
    }
}
void player::move()
{
    auto set_dash_state = [&](MOVE_DIRECTION dash) {
        if (dash_counter > 0 && from_last_dash <= 0) {
            if (dash_time < 0) {
                dash_time = 15;
                dash_counter--;
                dash_direction = dash;
                dashing = true;
                walking = false;
                keeping_grounded = on_ground;
                from_last_dash = 30;
                hyper_dashing = false;

                // 播放冲刺音效
                AudioManager& audioManager = AudioManager::getInstance();
                switch (rgen(0, 2)) {
                case 0:
                    audioManager.playSound("player_jump0");
                    break;
                case 1:
                    audioManager.playSound("player_jump1");
                    break;
                case 2:
                    audioManager.playSound("player_jump2");
                    break;
                }
            }
        }
    };
    collision();
    switch (direction) {
    case MOVE_DIRECTION::LEFT:
        std::cout << "left" << std::endl;
        // 加速
        if (!hyper_dashing && dash_time < 0) {
            velocity.x = -min(input_timer, 8) / 16.0 * MAX_SPEED;
            input_timer++;
            walking = true;
        }
        break;
    case MOVE_DIRECTION::RIGHT:
        std::cout << "right" << std::endl;
        // 加速
        if (!hyper_dashing && dash_time < 0) {
            velocity.x = min(input_timer, 8) / 16.0 * MAX_SPEED;
            input_timer++;
            walking = true;
        }
        break;
    case MOVE_DIRECTION::DASH_UP:
        std::cout << "dash up" << std::endl;
        set_dash_state(MOVE_DIRECTION::DASH_UP);
        break;
    case MOVE_DIRECTION::DASH_DOWN:
        std::cout << "dash down" << std::endl;
        set_dash_state(MOVE_DIRECTION::DASH_DOWN);
        break;
    case MOVE_DIRECTION::DASH_LEFT:
        std::cout << "dash left" << std::endl;
        set_dash_state(MOVE_DIRECTION::DASH_LEFT);
        break;
    case MOVE_DIRECTION::DASH_RIGHT:
        std::cout << "dash right" << std::endl;
        set_dash_state(MOVE_DIRECTION::DASH_RIGHT);
        break;
    case MOVE_DIRECTION::DASH_DOWN_LEFT:
        std::cout << "dash down left" << std::endl;
        set_dash_state(MOVE_DIRECTION::DASH_DOWN_LEFT);
        break;
    case MOVE_DIRECTION::DASH_DOWN_RIGHT:
        std::cout << "dash down right" << std::endl;
        set_dash_state(MOVE_DIRECTION::DASH_DOWN_RIGHT);
        break;
    case MOVE_DIRECTION::DASH_UP_LEFT:
        std::cout << "dash up left" << std::endl;
        set_dash_state(MOVE_DIRECTION::DASH_UP_LEFT);
        break;
    case MOVE_DIRECTION::DASH_UP_RIGHT:
        std::cout << "dash up right" << std::endl;
        set_dash_state(MOVE_DIRECTION::DASH_UP_RIGHT);
        break;
    default:
        std::cout << "idle" << std::endl;
        input_timer = 0;
        break;
    }
    // 冲刺处理
    if (keeping_grounded)
        on_ground = true;
    if (dash_time >= 0) {
        double dash_speed = max(dash_time * 3 - 12, 12);
        if (!hyper_dashing) {
            switch (dash_direction) {
            case MOVE_DIRECTION::DASH_UP:
                velocity = { 0.0, -1.0 * dash_speed };
                break;
            case MOVE_DIRECTION::DASH_DOWN:
                velocity = { 0.0, 1.0 * dash_speed };
                break;
            case MOVE_DIRECTION::DASH_LEFT:
                velocity = { -1.0 * dash_speed, 0.0 };
                break;
            case MOVE_DIRECTION::DASH_RIGHT:
                velocity = { 1.0 * dash_speed, 0.0 };
                break;
            case MOVE_DIRECTION::DASH_UP_LEFT:
                velocity = { -0.7 * dash_speed, -0.7 * dash_speed };
                break;
            case MOVE_DIRECTION::DASH_UP_RIGHT:
                velocity = { 0.7 * dash_speed, -0.7 * dash_speed };
                break;
            case MOVE_DIRECTION::DASH_DOWN_LEFT:
                velocity = { -0.7 * dash_speed, 0.7 * dash_speed };
                break;
            case MOVE_DIRECTION::DASH_DOWN_RIGHT:
                velocity = { 0.7 * dash_speed, 0.7 * dash_speed };
                break;
            default:
                break;
            }
        }
        dash_time--;
    } else {
        dash_time = -1;
        dashing = false;
        keeping_grounded = false;
        dash_direction = MOVE_DIRECTION::STOP;
    }
    if (from_last_dash > 0)
        from_last_dash--;
    // 跳跃处理
    if (jumping && can_jump()) {
        if (jump_timer < 6)
            jump_timer++;
        else {
            jump_timer = 0;
            velocity.y += -20.0;
            hyper_dashing = dash_time > 0 && (dash_direction != MOVE_DIRECTION::DASH_UP && dash_direction != MOVE_DIRECTION::DASH_UP_LEFT && dash_direction != MOVE_DIRECTION::DASH_UP_RIGHT && dash_direction != MOVE_DIRECTION::DASH_DOWN);
            if (hyper_dashing) {
                velocity.x = (velocity.x > 0.0 ? 1 : -1) * max(fabs(velocity.x), 24.0);
                pos.y -= 1;
                velocity.x *= 1.2;
                velocity.y *= 0.6;
            }
            jumping = false;
            jump_timer = 0;
            from_last_jump = 24;
        }
    } else if (jump_timer > 0 && can_jump()) {
        if (jump_timer > 6)
            velocity.y += -20.0;
        else
            velocity.y += -16.0;
        hyper_dashing = dash_time > 0 && (dash_direction != MOVE_DIRECTION::DASH_UP && dash_direction != MOVE_DIRECTION::DASH_UP_LEFT && dash_direction != MOVE_DIRECTION::DASH_UP_RIGHT && dash_direction != MOVE_DIRECTION::DASH_DOWN);
        if (hyper_dashing) {
            velocity.x = (velocity.x > 0.0 ? 1 : -1) * max(fabs(velocity.x), 24.0);
            velocity.x *= 1.2;
            pos.y -= 1;
            velocity.y *= 0.6;
        }
        from_last_jump = 24;
        jumping = false;
        jump_timer = 0;
    } else if (from_last_jump > 0) {
        from_last_jump--;
        // hyper_dashing = false;
    } else
        ;
    // 减速
    if (!jumping && !dashing) {
        if (on_ground) {
            if (fabs(velocity.x) > 0.1)
                velocity.x *= 0.75;
            else {
                velocity.x = 0.0;
                walking = false;
                hyper_dashing = false;
            }
        } else {
            walking = false;
            if (fabs(velocity.x) > 0.1)
                velocity.x *= 0.98;
            else
                velocity.x = 0.0;
        }
    }
    std::wcout << at_level->name() << std::endl;
    std::cout << "p: " << pos << ", v: " << velocity << ", ground: " << on_ground << std::endl;
    std::cout << "timer: " << input_timer << ", walking: " << walking << ", jumping: " << jumping << ", jump cooldown : " << from_last_jump << ", jump timer: " << jump_timer << ", can jump: " << can_jump() << std::endl;
    std::cout << "falling: " << falling_timer << ", dashing: " << dashing << ", dash cooldown : " << from_last_dash << ", dash count : " << dash_counter << ", time : " << dash_time << ", hyper : " << hyper_dashing << ", keeping ground : " << keeping_grounded << std::endl;
    std::cout << "death count: " << death_counter << std::endl;
    for (auto& l : level_entered) {
        std::wcout << l.first << ", " << l.second << std::endl;
    }
}
void player::trigger()
{
}
void player::checking_alive()
{
    if (invincible)
        hp = 99999;
    if (pos.y >= GAME_WINDOW_HEIGHT || pos.y < 0)
        hp = 0;
    if (hp == 0) {
        if (is_ironman) {
            AudioManager& audio = AudioManager::getInstance();
            audio.playSound("iron_man_fail");
            at_level->restart_game();
        }
        respawn();
    }
}
void player::draw_on(const float_pos& screen_pos)
{
    current_animation_frame++;
    reverse = (velocity.x == 0.0) ? reverse : (velocity.x < 0.0);
    if (!walking) {
        int animation_id = (current_animation_frame / frame_interval) % PLAYER_STAND_FRAME;
        putimage(int(screen_pos.x), int(screen_pos.y),
            img_width, img_height,
            &animation_list[2 + int(reverse)],
            0, animation_id * img_height, SRCAND);
        putimage(int(screen_pos.x), int(screen_pos.y),
            img_width, img_height,
            &animation_list[0 + int(reverse)],
            0, animation_id * img_height, SRCPAINT);
    } else if (walking) {
        int animation_id = (current_animation_frame / frame_interval) % PLAYER_WALK_FRAME;
        putimage(int(screen_pos.x), int(screen_pos.y),
            img_width, img_height,
            &animation_list[6 + int(reverse)],
            0, animation_id * img_height, SRCAND);
        putimage(int(screen_pos.x), int(screen_pos.y),
            img_width, img_height,
            &animation_list[4 + int(reverse)],
            0, animation_id * img_height, SRCPAINT);
    } else
        ;
    if (current_animation_frame > 60)
        current_animation_frame = 0;
}
void player::set_direction(MOVE_DIRECTION direct)
{
    direction = direct;
}
void player::respawn()
{
    pos = at_level->spawn_point();
    death_count(1);
    at_level->shift_animation();
    init();
    at_level->reset();
    // 播放死亡音效
    AudioManager& audioManager = AudioManager::getInstance();
    switch (rgen(0, 3)) {
    case 0:
        audioManager.playSound("player_death0");
        break;
    case 1:
        audioManager.playSound("player_death1");
        break;
    case 2:
        audioManager.playSound("player_death2");
        break;
    case 3:
        audioManager.playSound("player_death3");
        break;
    }
}
bool player::can_dash() const
{
    return !dashing && dash_counter > 0 && from_last_dash <= 0;
}
bool player::can_jump() const
{
    return from_last_jump <= 0 && (falling_timer <= 6 || on_ground);
}
void player::set_jumping(bool jump_state)
{
    jumping = jump_state;
}
void player::death_count(int add)
{
    death_counter += add;
}
int player::death_time() const
{
    return death_counter;
}
int player::append_data()
{
    return int(is_ironman) + (power_point == 2 ? 2 : 0);
}

platform::platform(std::shared_ptr<game_level> level,
    const float_pos& place_pos, int width, int height, bool dangerous)
{
    at_level = level;
    is_static = true;
    is_dangerous = dangerous;
    collapsing = false;
    visible = false;
    collide = true;
    collapse_timer = 0;
    from_last_collapse = 0;
    pos = place_pos;
    collide_box.x = width;
    collide_box.y = height;
    hp = 1;
    if (is_dangerous) {
        load_image_from_path(&animation_list[0], L"asset\\image\\unstable_platform.png", 60, height * 6, true);
        load_image_from_path(&animation_list[1], L"asset\\image\\unstable_platform_mask.png", 60, height * 6, true);
    }
    load_image_from_path(&animation_list[2], L"asset\\image\\platform.png", width, height, 1);
    img_height = img.getheight();
    img_width = img.getwidth();
    power_point = 0;
    velocity = { 0.0, 0.0 };
    current_animation_frame = 0;
    frame_interval = 0;
    type = OBJECT_TYPE::PLATFORM;
}
void platform::draw_on(const float_pos& screen_pos)
{
    if (visible) {
        if (is_dangerous) {
            for (int i = 0; i < collide_box.x / 60; ++i) {
                putimage(int(screen_pos.x) + i * 60, int(screen_pos.y), 60, int(collide_box.y),
                    &animation_list[1], 0, current_animation_frame * 60, SRCAND);
                putimage(int(screen_pos.x) + i * 60, int(screen_pos.y), 60, int(collide_box.y),
                    &animation_list[0], 0, current_animation_frame * 60, SRCPAINT);
            }
        } else {
            // putimage(int(screen_pos.x), int(screen_pos.y), int(collide_box.x), int(collide_box.y),&animation_list[2], 0, 0);
        }
    }
}
void platform::trigger()
{
    if (!is_dangerous || collapsing)
        return;
    collapsing = true;
    collapse_timer = PLATFORM_COLLAPSE_FRAME;
}
void platform::update()
{
    if (!is_dangerous) {
        visible = true;
        return;
    } else
        visible = true;
    if (collapse_timer > 0) {
        collapse_timer--;
        collapsing = true;
        current_animation_frame = (PLATFORM_COLLAPSE_FRAME - collapse_timer) * 6 / PLATFORM_COLLAPSE_FRAME;
        collide = true;
        visible = true;
    } else if (collapsing) {
        from_last_collapse = PLATFORM_COLLAPSE_FRAME;
        visible = false;
        collapsing = false;
        collide = false;
    } else
        ;
    if (from_last_collapse > 0) {
        from_last_collapse--;
        collide = false;

        if (from_last_collapse > PLATFORM_COLLAPSE_FRAME / 2) {
            current_animation_frame = 0;
            visible = false;
        } else {
            current_animation_frame = from_last_collapse * 12 / PLATFORM_COLLAPSE_FRAME;
            visible = true;
        }
    } else {
        visible = true;
        collide = true;
    }
}

gate::gate(std::shared_ptr<game_level> level,
    const float_pos& place_pos, int width, int height)
{
    at_level = level;
    is_static = true;
    visible = true;
    collide = true;
    pos = place_pos;
    original_pos = pos;
    open_timer = -1;
    collide_box = float_pos(double(width), double(height));
    img_width = width;
    img_height = height;
    type = OBJECT_TYPE::PLATFORM;
    load_image_from_path(&img, L"asset\\image\\cobblestone.png", 0, 0, false);
}
void gate::update()
{
    if (open_timer == -1 && !at_level->has(OBJECT_TYPE::ENEMY)) {
        open_timer = 120;
    }
    if (open_timer > 0) {
        pos = original_pos + float_pos(0.5 * (120 - open_timer), 0);
        open_timer--;
    } else if (open_timer != -1) {
        hp = 0;
    }
}
void gate::draw_on(const float_pos& screen_pos)
{
    for (int x = 0; x < img_width / 60; x++) {
        for (int y = 0; y < img_height / 60; y++) {
            putimage(int(x * 60 + screen_pos.x), int(y * 60 + screen_pos.y), &img);
        }
    }
}
void gate::trigger()
{
}

level_trigger::level_trigger(const float_pos& place_pos, const float_pos& size,
    std::shared_ptr<game_level> at_level, const std::wstring& next_level_identifier,
    TRIGGER_TYPE trigger_type, const float_pos& land_pos)
    : at_level(at_level)
    , next_level_identifier(next_level_identifier)
    , next_level(nullptr)
    , trigger_type(trigger_type)
    , land_pos(land_pos)
{
    pos = place_pos;
    collide_box = size;
    type = OBJECT_TYPE::TRIGGER;
    // load_image_from_path(&img, L"asset\\image\\default.png", int(size.x), int(size.y), true);
    collide = false;
}
void level_trigger::update()
{
    auto list = at_level->collide_with(shared_from_this());
    if (list.empty())
        return;
    else {
        for (auto& obj : list) {
            if (obj->get_type() == OBJECT_TYPE::PLAYER) {
                next_level = at_level->get_level_from_name(next_level_identifier);
                switch (trigger_type) {
                case TRIGGER_TYPE::BOUND:
                    at_level->change_level(next_level, land_pos);
                    break;
                case TRIGGER_TYPE::SPEED:
                    if (fabs(obj->get_velocity().x) >= 26.0)
                        at_level->change_level(next_level, land_pos);
                    break;
                case TRIGGER_TYPE::END:
                    at_level->shift_animation(true);
                    break;
                }
            }
        }
    }
}

collection_ironman::collection_ironman(const float_pos& place_pos,
    std::shared_ptr<game_level> level)
{
    at_level = level;
    pos = place_pos;
    original_pos = pos;
    collide_box = { 64.0, 64.0 };
    img_width = img_height = 64;
    type = OBJECT_TYPE::COLLECTION;
    load_image_from_path(&img, L"asset/textures/item/ironman.png", 0, 0, false);
}
void collection_ironman::update()
{
    auto collide = at_level->collide_with(shared_from_this());
    if (!collide.empty()) {
        for (auto& obj : collide) {
            if (obj->get_type() == OBJECT_TYPE::PLAYER) {
                AudioManager& audioManager = AudioManager::getInstance();
                audioManager.playMusic("battle_start");
                auto obj_player = static_cast<player*>(obj.get());
                obj_player->is_ironman = true;
                hp = 0;

                break;
            }
        }
    }
}
void collection_ironman::draw_on(const float_pos& screen_pos)
{
    current_animation_frame++;
    pos = original_pos.offset(float_pos(0, sin(current_animation_frame * 0.03) * 16));
    putimage(int(pos.x), int(pos.y), &img, SRCPAINT);
}

collection_weapon::collection_weapon(const float_pos& place_pos,
    std::shared_ptr<game_level> level)
{
    at_level = level;
    pos = place_pos;
    original_pos = pos;
    collide_box = { 64.0, 64.0 };
    img_width = img_height = 64;
    type = OBJECT_TYPE::COLLECTION;
    load_image_from_path(&img, L"asset/textures/item/weapon.png", 0, 0, false);
}
void collection_weapon::update()
{
    auto collide = at_level->collide_with(shared_from_this());
    if (!collide.empty()) {
        for (auto& obj : collide) {
            if (obj->get_type() == OBJECT_TYPE::PLAYER) {
                auto obj_player = static_cast<player*>(obj.get());
                obj_player->power_point = 2;
                hp = 0;
                break;
            }
        }
    }
}
void collection_weapon::draw_on(const float_pos& screen_pos)
{
    current_animation_frame++;
    pos = original_pos.offset(float_pos(0, sin(current_animation_frame * 0.03) * 16));
    putimage(int(pos.x), int(pos.y), &img, SRCPAINT);
}

spike::spike(std::shared_ptr<game_level> level,
    const float_pos& place_pos, int width, int height, MOVE_DIRECTION direct)
{
    at_level = level;
    is_static = true;
    pos = place_pos;
    collide_box.x = width;
    collide_box.y = height;
    hp = 1;
    // load_image_from_path(&img, L"asset\\image\\default.png", width, height, true);
    img_height = img.getheight();
    img_width = img.getwidth();
    power_point = 0;
    velocity = { 0.0, 0.0 };
    current_animation_frame = 0;
    frame_interval = 0;
    collide = false;
    type = OBJECT_TYPE::SPIKE;
    direction = direct;
}
void spike::update()
{
    auto list = at_level->collide_with(shared_from_this());
    if (list.empty())
        return;
    else {
        for (auto& obj : list) {
            if (obj->get_type() == OBJECT_TYPE::PLAYER) {
                switch (direction) {
                case MOVE_DIRECTION::UP:
                    if (obj->get_velocity().y < 0.0)
                        return;
                    break;
                case MOVE_DIRECTION::DOWN:
                    if (obj->get_velocity().y > 0.0)
                        return;
                    break;
                case MOVE_DIRECTION::LEFT:
                    if (obj->get_velocity().x < 0.0)
                        return;
                    break;
                case MOVE_DIRECTION::RIGHT:
                    if (obj->get_velocity().x > 0.0)
                        return;
                    break;
                }
                obj->set_hp(0);
                return;
            }
        }
    }
}

void boss1::damaku1() const
{
    AudioManager& audioManager = AudioManager::getInstance();
    audioManager.playSound("bullet");
    // 以自身为原点发射圆形弹幕
    int total = min(attack_stage + 12, 360);
    for (int i = 0; i < total; ++i) {
        float_pos v = position(cos(attack_stage * -0.15 + i / (total * 0.5) * std::numbers::pi), sin(attack_stage * -(0.15) + i / (total * 0.5) * std::numbers::pi)) * 10;
        at_level->add_object_request(std::make_shared<damaku_bullet>(at_level, -1, get_center(), v));
    }
}
void boss1::damaku2() const
{
    // 向玩家发射激光
    float_pos direction = at_level->get_player()->get_center() - get_center();
    double angle = atan2(direction.y, direction.x);
    at_level->add_object_request(std::make_shared<damaku_lazer>(at_level, 120, get_center(), angle, true));
}
boss1::boss1(const std::shared_ptr<game_level>& level, const float_pos& p)
{
    pos = p;
    hp = 1;
    img_width = 128;
    img_height = 128;
    collide_box = float_pos(128.0, 128.0);
    at_level = level;
    type = OBJECT_TYPE::ENEMY;
    load_image_from_path(&img_list[0], L"asset\\textures\\mob\\boss_1.png", 0, 0, false);
    load_image_from_path(&img_list[1], L"asset\\textures\\mob\\boss_1_mask.png", 0, 0, false);
    from_last_attack = 60;
    attack_stage = 0;
    death_timer = -1;
    type = OBJECT_TYPE::ENEMY;
}
void boss1::update()
{
    if (from_last_attack)
        from_last_attack--;
    else {
        if (attack_stage < 10) {
            damaku1();
            from_last_attack = 30;
        } else if (attack_stage < 15) {
            damaku2();
            from_last_attack = 90;
        } else if (attack_stage < 20) {
            damaku2();
            damaku1();
            from_last_attack = 60;
        } else {
            death_timer = 300;
            from_last_attack = 99999;
        }
        attack_stage++;
    }
    if (death_timer > 0) {
        death_timer--;
        pos.delta(rgen(-3, 3), rgen(-3, 3));
    } else if (death_timer == 0) {
        at_level->add_object(std::make_shared<collection_weapon>(float_pos(120.0, 420.0), at_level));
        hp = 0;
    } else
        ;
}
void boss1::draw_on(const float_pos& screen_pos)
{
    current_animation_frame++;
    putimage(int(screen_pos.x), int(screen_pos.y), img_width, img_height,
        &img_list[1], 0, 128 * ((current_animation_frame / 30) % 4), SRCAND);
    putimage(int(screen_pos.x), int(screen_pos.y), img_width, img_height,
        &img_list[0], 0, 128 * ((current_animation_frame / 30) % 4), SRCPAINT);
}
void boss1::trigger()
{
}

void boss2::damaku() const
{
    float_pos direction = at_level->get_player()->get_center() - get_center();
    double angle;
    if (direction.y > std::fabs(direction.x))
        angle = -std::numbers::pi * 0.5;
    else if (direction.y < -std::fabs(direction.x))
        angle = std::numbers::pi * 0.5;
    else if (direction.x > std::fabs(direction.y))
        angle = 0;
    else
        angle = -std::numbers::pi;
    at_level->add_object_request(std::make_shared<damaku_lazer>(at_level, 30, get_center(), angle, true));
}
boss2::boss2(const std::shared_ptr<game_level>& level, const float_pos& p)
{
    at_level = level;
    pos = p;
    next_pos = pos;
    from_last_attack = 120;
    load_image_from_path(&img_list[0], L"asset\\textures\\mob\\boss_2.png", 128, 512, true);
    load_image_from_path(&img_list[1], L"asset\\textures\\mob\\boss_2_mask.png", 128, 512, true);
    img_width = 128;
    img_height = 128;
    powering = false;
    fire_timer = -1;
    hp = 5;
    laser_timer = -1;
    invincible = false;
    invincible_timer = -1;
    collide_box = { 128.0, 128.0 };
    visible = true;
    type = OBJECT_TYPE::ENEMY;
}
void boss2::update()
{
    auto to_grid_pos = [](const int_pos& p) -> float_pos {
        return { p.x * 60.0 - 34.0, p.y * 60.0 - 34.0 };
    };
    auto grid_center_pos = [](const int_pos& p) -> float_pos {
        return { p.x * 60.0 + 30.0, p.y * 60.0 + 30.0 };
    };
    auto anxious = [](int distance) -> bool {
        return distance < 600;
    };
    auto anxiety = [](int distance) -> int {
        return distance > 1600 ? 20 : (distance > 1200 ? 15 : 10);
    };
    from_last_attack--;
    float_pos self_center = get_center(), player_center = at_level->get_player()->get_center();
    int distance = int((player_center - self_center).mod());
    if (from_last_attack == 0) {
        at_grid = { int(self_center.x / 60), int(self_center.y / 60) };
        if (!anxious(distance)) {
            int_pos player_grid = { int(player_center.x / 60), int(player_center.y / 60) };
            auto route = at_level->route(at_grid, player_grid);
            if (!route.empty())
                goal = std::move(route);
        } else {
            int_pos target(rgen(1, MAZE_WIDTH - 2), rgen(1, MAZE_HEIGHT - 2));
            while (true) {
                if (at_level->maze_map(target) && (grid_center_pos(target) - pos).mod() >= 600)
                    break;
                target = int_pos(rgen(1, MAZE_WIDTH - 2), rgen(1, MAZE_HEIGHT - 2));
            }
            auto route = at_level->route(at_grid, target);
            if (!route.empty())
                goal = std::move(route);
        }
        from_last_attack = hp > 2 ? 120 : 60;
    }
    if (!powering) {
        if (from_last_attack % anxiety(distance) == 0) {
            if (!goal.empty()) {
                int_pos tmp_pos = goal.front();
                goal.pop();
                if (!goal.empty()) {
                    int_pos delta = goal.front() - tmp_pos;
                    while (!goal.empty() && (goal.front() - tmp_pos) == delta && (grid_center_pos(goal.front()) - player_center).mod() > 1000) {
                        tmp_pos = goal.front();
                        goal.pop();
                    }
                }
                next_pos = to_grid_pos(tmp_pos);
            }
        }
        if ((next_pos - pos).mod() > 1)
            pos = pos + (next_pos - pos) * 0.1;
        else
            pos = next_pos;
    }
    if (!invincible) {
        if (!powering && fire_timer == -1 && laser_timer <= 0 && (std::fabs(player_center.x - self_center.x) < 60 || std::fabs(player_center.y - self_center.y) < 60)) {
            powering = true;
            fire_timer = 60;
        }
        if (fire_timer > 0)
            fire_timer--;
        else if (fire_timer != -1) {
            powering = false;
            from_last_attack = 120;
            fire_timer = -1;
            laser_timer = 30;
            damaku();
        } else
            ;
    } else
        ;
    if (laser_timer > 0)
        laser_timer--;
    if (!invincible) {
        auto hit = at_level->collide_with(shared_from_this());
        if (!hit.empty()) {
            for (auto& obj : hit) {
                if (obj->get_type() == OBJECT_TYPE::PLAYER && obj->get_velocity().mod() >= 22.0 && !invincible) {
                    AudioManager& audio = AudioManager::getInstance();
                    audio.playSound("boss_hit");
                    invincible = true;
                    invincible_timer = 120;
                    visible = true;
                    attack(obj->get_power_point());
                    if (hp < 3) {
                        AudioManager& audioManager = AudioManager::getInstance();
                        audioManager.playMusic("battle_start");
                    }
                    break;
                }
            }
        }
    }
    if (invincible_timer > 0) {
        invincible_timer--;
        invincible = true;
        if (invincible_timer % (invincible_timer > 40 ? 6 : 3) == 0)
            visible = !visible;
    } else if (invincible_timer != -1) {
        visible = true;
        invincible = false;
        invincible_timer = -1;
        int_pos target(rgen(1, MAZE_WIDTH - 2), rgen(1, MAZE_HEIGHT - 2));
        while (true) {
            if (at_level->maze_map(target) && (grid_center_pos(target) - pos).mod() >= 300)
                break;
            target = int_pos(rgen(1, MAZE_WIDTH - 2), rgen(1, MAZE_HEIGHT - 2));
        }
        pos = to_grid_pos(target);
        goal = {};
        next_pos = pos;
    } else
        ;
}
void boss2::draw_on(const float_pos& screen_pos)
{
    current_animation_frame++;
    if (!visible)
        return;
    int frame_id = (current_animation_frame / 15) % 4;
    putimage(int(screen_pos.x), int(screen_pos.y), img_width, img_height,
        &img_list[1], 0, frame_id * img_height, SRCAND);
    putimage(int(screen_pos.x), int(screen_pos.y), img_width, img_height,
        &img_list[0], 0, frame_id * img_height, SRCPAINT);
}
void boss2::trigger()
{
}

boss3::boss3(const std::shared_ptr<game_level>& level, const float_pos& p)
{
    at_level = level;
    pos = p;
    from_last_attack = 60;
    hp = 3;
    invincible = false;
    invincible_timer = -1;
    bullet_timer = 0;
    laser_timer = 0;
    visible = true;
    bullet_count = 1;
    holding_bullet = nullptr;
    type = OBJECT_TYPE::ENEMY;
    load_image_from_path(&img_list[0], L"asset\\textures\\mob\\final_boss.png", 96, 768, false);
    load_image_from_path(&img_list[1], L"asset\\textures\\mob\\final_boss_mask.png", 96, 768, false);
    img_height = 96;
    img_width = 96;
    collide_box = { 96.0, 96.0 };
}
void boss3::damaku1()
{
    AudioManager& audio = AudioManager::getInstance();
    audio.playSound("bullet");
    holding_bullet = std::make_shared<boss3_bullet>(at_level, 9999999, pos);
    at_level->add_object(holding_bullet);
}
void boss3::damaku2() const
{
    if (hp == 1) {
        int player_grid_pos = int(floor(at_level->get_player()->get_center().x / 60));
        for (int i = 0; i < player_grid_pos - 5; ++i) {
            if (rgen(1, 5) == 1)
                at_level->add_object(std::make_shared<damaku_lazer>(at_level, 60, float_pos(i * 60 + 30.0, 0), -std::numbers::pi * 0.5, false));
        }
        if (rgen(1, 3) == 1) {
            at_level->add_object(std::make_shared<damaku_lazer>(at_level, 60, float_pos(floor(at_level->get_player()->get_center().x / 60) * 60 + 30.0, 0), -std::numbers::pi * 0.5, false));
            at_level->add_object(std::make_shared<damaku_lazer>(at_level, 60, float_pos(0, floor(at_level->get_player()->get_center().y / 60) * 60 + 30.0), 0, false));
        }
    }
    at_level->add_object(std::make_shared<damaku_lazer>(at_level, 60, get_center(), 0, true));
}
void boss3::update()
{
    auto grid_center_pos = [](const int_pos& pos) -> float_pos {
        return float_pos(pos.x * 60 + 30.0, pos.y * 60 + 30.0);
    };
    auto to_grid_pos = [](const int_pos& pos) -> float_pos {
        return float_pos(pos.x * 60 - 18.0, pos.y * 60 - 18.0);
    };
    auto collide = at_level->collide_with(shared_from_this());
    if (!invincible && invincible_timer == -1 && !collide.empty()) {
        for (auto& obj : collide) {
            if (obj->get_type() == OBJECT_TYPE::PLAYER && obj->get_velocity().mod() > 24.0) {
                AudioManager& audio = AudioManager::getInstance();
                audio.playSound("boss_hit");
                attack(1);
                if (holding_bullet != nullptr) {
                    holding_bullet->set_hp(0);
                    holding_bullet = nullptr;
                }
                invincible = true;
                break;
            }
        }
    }
    if (invincible && invincible_timer == -1) {
        invincible_timer = 120;
        invincible = false;
        visible = true;
    }
    if (invincible_timer > 0) {
        invincible_timer--;
        if (invincible_timer % (invincible_timer > 60 ? 20 : 10) == 0)
            visible = !visible;
    } else if (invincible_timer != -1) {
        invincible_timer = -1;
        invincible = false;
        visible = true;
        int_pos target;
        if (hp == 2)
            target = { 15, 10 };
        else if (hp == 1)
            target = { 27, 3 };
        else
            ;
        pos = to_grid_pos(target);
        goal = {};
        next_pos = pos;
    } else
        ;
    if (bullet_timer > 0)
        bullet_timer--;
    if (laser_timer > 0)
        laser_timer--;
    if (!invincible && invincible_timer == -1) {
        if (bullet_count > 0 && bullet_timer == 0 && hp == 3) {
            bullet_timer = 120;
            damaku1();
            bullet_count--;
        }
        if (laser_timer == 0 && hp < 3) {
            laser_timer = (hp > 1 ? 120 : 60);
            damaku2();
        }
    }
    printf("%d %d %d %d %d %d\n", invincible, invincible_timer, visible, laser_timer, bullet_timer, hp);
}
void boss3::draw_on(const float_pos& screen_pos)
{
    current_animation_frame++;
    int frame_id = (current_animation_frame / 15) % 8;
    if (!visible)
        return;
    putimage(int(screen_pos.x), int(screen_pos.y), img_width, img_height,
        &img_list[1], 0, frame_id * img_height, SRCAND);
    putimage(int(screen_pos.x), int(screen_pos.y), img_width, img_height,
        &img_list[0], 0, frame_id * img_height, SRCPAINT);
}
void boss3::trigger()
{
}

damaku_bullet::damaku_bullet(const std::shared_ptr<game_level>& level, int life, const float_pos& at, const float_pos& init_velocity)
{
    lifetime = life;
    at_level = level;
    collide_box = float_pos(10.0, 10.0);
    img_height = 16;
    img_width = 16;
    pos = at;
    type = OBJECT_TYPE::BULLET;
    velocity = init_velocity;
    load_image_from_path(&img_list[0], L"asset\\textures\\mob\\bullet_boss_1.png", 16, 16, true);
    load_image_from_path(&img_list[1], L"asset\\textures\\mob\\bullet_boss_1_mask.png", 16, 16, true);
}
void damaku_bullet::update()
{
    pos.delta(velocity);
    if (pos.x < 0 || pos.x > GAME_WINDOW_WIDTH || pos.y < 0 || pos.y > GAME_WINDOW_HEIGHT)
        hp = 0;
    auto list = at_level->collide_with(shared_from_this());
    if (velocity.mod() > 5) {
        velocity = velocity * 0.95;
    } else {
        velocity = velocity.unify() * 5;
    }
    if (!list.empty()) {
        for (auto& obj : list) {
            if (obj->get_type() == OBJECT_TYPE::PLAYER) {
                obj->attack(1);
                break;
            }
        }
    }
}
void damaku_bullet::draw_on(const float_pos& screen_pos)
{
    putimage(int(screen_pos.x), int(screen_pos.y), &img_list[1], SRCAND);
    putimage(int(screen_pos.x), int(screen_pos.y), &img_list[0], SRCPAINT);
}
void damaku_bullet::trigger()
{
}

damaku_lazer::damaku_lazer(const std::shared_ptr<game_level>& level,
    int life, const float_pos& p, double ang, bool aim)
{
    at_level = level;
    lifetime = life;
    pos = p;
    facing_angle = ang;
    aiming = aim;
    visible = true;
    load_image_from_path(&lazer_img[0], L"asset\\image\\aim_boss_2.png", 0, 0, false);
    load_image_from_path(&lazer_img[1], L"asset\\image\\laser_boss_2.png", 0, 0, false);
}
void damaku_lazer::update()
{
    AudioManager& audioManager = AudioManager::getInstance();
    lifetime--;
    if (lifetime > 20) {
        if (aiming) {
            float_pos direction = at_level->get_player()->get_center() - get_center();
            facing_angle = atan2(-direction.y, direction.x);
        }
        visible = true;
    } else if (lifetime > 0) {
        if (lifetime == 20)
            audioManager.playSound("laser_powering");
    } else if (lifetime > -20) {
        visible = true;
        if (lifetime == 0)
            audioManager.playSound("laser_fire");
        // 检查伤害
        if (lifetime < -3) {
            float_pos player_pos = at_level->get_player()->get_center(),
                      self_pos = get_center();
            double mod = (player_pos - self_pos).mod();
            float_pos direct_pos = (player_pos - self_pos).unify(),
                      delt_pos(cos(facing_angle), sin(facing_angle));
            direct_pos.y *= -1;
            double prod = delt_pos.dot_product(direct_pos),
                   theta = acos(prod);
            double vdistance = fabs(sin(theta)) * mod;
            if (prod > 0 && vdistance < 15)
                at_level->get_player()->set_hp(0);
        }
    } else {
        hp = 0;
    }
}
void damaku_lazer::draw_on(const float_pos& screen_pos)
{
    if (!visible)
        return;
    current_animation_frame++;
    if (current_animation_frame % (lifetime > 20 ? 6 : 2) != 0)
        return;
    IMAGE tmp_img;
    if (lifetime > 0)
        rotateimage(&tmp_img, &lazer_img[0], facing_angle, BLACK, true, false);
    else
        rotateimage(&tmp_img, &lazer_img[1], facing_angle, BLACK, true, false);
    bool x_rev = fabs(facing_angle) > std::numbers::pi * 0.5;
    bool y_rev = facing_angle > 0;
    int put_x = int(screen_pos.x + (x_rev ? -tmp_img.getwidth() : 0)),
        put_y = int(screen_pos.y + (y_rev ? -tmp_img.getheight() : 0));
    putimage(put_x, put_y, &tmp_img, SRCINVERT);
}
void damaku_lazer::trigger()
{
}

boss3_bullet::boss3_bullet(const std::shared_ptr<game_level>& level, int life, const float_pos& p)
{
    at_level = level;
    lifetime = life;
    pos = p;
    next_pos = pos;
    collide_box = float_pos(22.0, 32.0);
    repath_timer = 0;
    visible = true;
    load_image_from_path(&img_list[0], L"asset\\textures\\mob\\final_boss_bullet.png", 0, 0, false);
    load_image_from_path(&img_list[1], L"asset\\textures\\mob\\final_boss_bullet_mask.png", 0, 0, false);
}
void boss3_bullet::update()
{
    lifetime--;
    if (lifetime < 60) {
        if (lifetime % 10 == 0)
            visible = !visible;
        if (lifetime == 0)
            hp = 0;
    }
    auto collide = at_level->collide_with(shared_from_this());
    if (!collide.empty()) {
        for (auto& obj : collide) {
            if (obj->get_type() == OBJECT_TYPE::PLAYER) {
                at_level->get_player()->attack(1);
                return;
            }
        }
    }
    if (repath_timer == 0) {
        auto to_grid_pos = [](const int_pos& p) {
            return float_pos(p.x * 60 + 19.0, p.y * 60 + 16.0);
        };
        float_pos center = get_center(),
                  pcenter = at_level->get_player()->get_center();
        int_pos grid_pos(int(center.x / 60), int(center.y / 60)),
            grid_player(int(pcenter.x / 60), int(pcenter.y / 60));
        route = std::move(at_level->route(grid_pos, grid_player));
        if (!route.empty())
            route.pop();
        if (!route.empty()) {
            next_pos = to_grid_pos(route.front());
            route.pop();
        }
        repath_timer = 20;
    } else {
        repath_timer--;
        if ((next_pos - pos).mod() > 1)
            pos = pos + (next_pos - pos) * 0.1;
        else
            pos = next_pos;
    }
}
void boss3_bullet::draw_on(const float_pos& screen_pos)
{
    if (!visible)
        return;
    putimage(int(screen_pos.x), int(screen_pos.y), &img_list[1], SRCAND);
    putimage(int(screen_pos.x), int(screen_pos.y), &img_list[0], SRCPAINT);
}
void boss3_bullet::trigger()
{
}
