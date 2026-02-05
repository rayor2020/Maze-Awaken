#pragma once
#include "../utils/maze_utils.h"
#include "../utils/position.h"
#include <easyx.h>
#include <map>
#include <queue>
#include <string>
#include <vector>
static int global_object_id = 0;
class game_level;
class game_object : public std::enable_shared_from_this<game_object> {
protected:
    bool is_static; // 区分可交互实体
    float_pos pos; // 位置(左下角)
    float_pos collide_box; // 碰撞箱大小
    float_pos collide_delta; // 碰撞箱偏移量
    bool collide; // 是否参与碰撞检测
    int hp; // 血量
    int power_point; // 攻击力
    float_pos velocity; // 速度
    IMAGE img; // 贴图路径
    int img_height; // 贴图高度
    int img_width; // 贴图宽度
    int total_animation_frame; // 动画帧总数
    int current_animation_frame; // 当前动画帧
    int frame_interval; // 动画帧间隔(每n帧)
    std::shared_ptr<game_level> at_level; // 所处关卡
    int id; // 对象id
    OBJECT_TYPE type; // 对象类型
public:
    static void load_image_from_path(IMAGE* img, const std::wstring& path, int set_height = 0, int set_width = 0, bool resize = false)
    {
        int status = loadimage(img, path.c_str(), set_height, set_width, resize);
        if (status != NO_ERROR)
            status = loadimage(img, L"assets\\textures\\default.png", set_height, set_width, resize);
        // std::wcout << L"Loading from path " << path << ": " << status << std::endl;
    }

public:
    float_pos get_pos() const;
    void set_pos(double x, double y);
    void set_pos(const float_pos& p);
    void set_hp(int new_hp);
    void set_level(std::shared_ptr<game_level> new_level);
    float_pos get_velocity() const;
    OBJECT_TYPE get_type() const;
    float_pos get_hitbox() const;
    float_pos get_center() const;
    int get_hp() const;
    int get_power_point() const;
    float_pos get_hitbox_delta() const;
    int get_id() const;
    bool is_physic() const;
    void attack(int damage);

public:
    game_object(
        const float_pos& pos = float_pos(),
        const float_pos& collide_box = float_pos(),
        bool collide = true,
        int hp = 1,
        int power_point = 1,
        int img_height = 0,
        int img_width = 0,
        const float_pos& velocity = float_pos(),
        bool is_static = false,
        int frame_interval = 1);
    virtual void update() = 0;
    virtual void draw_on(const float_pos& screen_pos);
    virtual void trigger() = 0;
    virtual int append_data();
    friend class game_level;
    friend class GameMain;
};
class player : public game_object {
private:
    bool on_ground; // 触地状态
    bool dashing; // 冲刺状态
    int dash_time; // 玩家冲刺计时器
    int dash_counter; // 玩家冲刺次数
    bool jumping; // 跳跃状态
    bool walking; // 走路状态
    bool reverse; // 贴图反转状态
    int from_last_jump; // 跳跃冷却计时器
    int from_last_dash; // 冲刺冷却计时器
    int input_timer; // 键盘输入计时器
    int jump_timer; // 跳跃输入计时器
    int falling_timer; // 下落计时器
    bool hyper_dashing; // hyper状态
    bool keeping_grounded; // 保持触地状态
    IMAGE animation_list[PLAYER_ANIMATION_TOTAL * 4]; // 玩家动画列表
    MOVE_DIRECTION direction; // 运动方向
    MOVE_DIRECTION dash_direction; // 冲刺方向
    float_pos dash_velocity; // 冲刺速度
    int death_counter; // 死亡计数器
    bool is_ironman; // 是否一命通关
    bool invincible; // 是否无敌
    std::map<std::wstring, bool> level_entered;

protected:
    bool can_dash() const;
    bool can_jump() const;
    void init(bool is_restart = false);
    void set_direction(MOVE_DIRECTION direct);
    void set_jumping(bool jump_state);
    void checking_alive();
    void death_count(int add);
    void move();
    void respawn();
    void collision();
    int death_time() const;

public:
    void set_is_ironman(bool new_state);

    player(std::shared_ptr<game_level> level);
    void update() override;
    void draw_on(const float_pos& screen_pos) override;
    void trigger() override;
    int append_data() override;
    friend class GameMain;
    friend class collection_ironman;
    friend class collection_weapon;
};

class damaku_bullet : public game_object {
private:
    int lifetime;
    IMAGE img_list[2];

public:
    damaku_bullet(const std::shared_ptr<game_level>& at_level, int lifetime, const float_pos& pos, const float_pos& velocity);
    void update() override;
    void draw_on(const float_pos& screen_pos) override;
    void trigger() override;
};
class damaku_lazer : public game_object {
private:
    int lifetime;
    double facing_angle;
    bool visible;
    IMAGE lazer_img[2];
    bool aiming;

public:
    damaku_lazer(const std::shared_ptr<game_level>& at_level,
        int lifetime, const float_pos& pos, double angle, bool aiming = true);
    void update() override;
    void draw_on(const float_pos& screen_pos) override;
    void trigger() override;
};

class boss3_bullet : public game_object {
private:
    int lifetime;
    IMAGE img_list[2];
    int repath_timer;
    float_pos next_pos;
    std::queue<int_pos> route;
    bool visible;

public:
    boss3_bullet(const std::shared_ptr<game_level>& at_level, int lifetime, const float_pos& pos);
    void update() override;
    void draw_on(const float_pos& screen_pos) override;
    void trigger() override;
};

class boss1 : public game_object {
private:
    int from_last_attack;
    int attack_stage;
    int death_timer;
    IMAGE img_list[2];

private:
    void damaku1() const;
    void damaku2() const;

public:
    boss1(const std::shared_ptr<game_level>& at_level, const float_pos& pos);
    void update() override;
    void draw_on(const float_pos& screen_pos) override;
    void trigger() override;
};

class boss2 : public game_object {
private:
    int from_last_attack; // 攻击计时器
    int_pos at_grid; // 所处节点
    IMAGE img_list[2]; // 图像列表
    std::queue<int_pos> goal; // 目标路径
    float_pos next_pos; // 目标点
    bool powering; // 蓄能状态
    int fire_timer; // 开火计时器
    int laser_timer; // 激光计时器
    bool invincible; // 无敌状态
    int invincible_timer; // 无敌倒计时
    bool visible; // 可见状态
private:
    void damaku() const;

public:
    boss2(const std::shared_ptr<game_level>& at_level, const float_pos& pos);
    void update() override;
    void draw_on(const float_pos& screen_pos) override;
    void trigger() override;
};

class boss3 : public game_object {
private:
    int from_last_attack; // 攻击计时器
    int_pos at_grid; // 所处节点
    IMAGE img_list[2]; // 图像列表
    std::queue<int_pos> goal; // 目标路径
    float_pos next_pos; // 目标点
    bool invincible; // 无敌状态
    int invincible_timer; // 无敌倒计时
    bool visible; // 可见状态
    int laser_timer; // 激光计时器
    int bullet_timer; // 子弹计时器
    int bullet_count; // 子弹总数
    std::shared_ptr<boss3_bullet> holding_bullet; // 子弹指针
private:
    void damaku1();
    void damaku2() const;

public:
    boss3(const std::shared_ptr<game_level>& at_level, const float_pos& pos);
    void update() override;
    void draw_on(const float_pos& screen_pos) override;
    void trigger() override;
};

class platform : public game_object {
private:
    bool is_dangerous;
    int from_last_collapse;
    int collapse_timer;
    bool collapsing;
    IMAGE animation_list[3];
    bool visible;

public:
    platform(std::shared_ptr<game_level> level,
        const float_pos& place_pos, int width, int height, bool dangerous);
    void update() override;
    void draw_on(const float_pos& screen_pos) override;
    void trigger();
};

class gate : public game_object {
private:
    int open_timer;
    bool visible;
    float_pos original_pos;

public:
    gate(std::shared_ptr<game_level> level,
        const float_pos& place_pos, int width, int height);
    void update() override;
    void draw_on(const float_pos& screen_pos) override;
    void trigger();
};

class spike : public game_object {
private:
    MOVE_DIRECTION direction;

public:
    spike(std::shared_ptr<game_level> level,
        const float_pos& place_pos, int width, int height, MOVE_DIRECTION direct);
    void update() override;
    void trigger() override { }
};

class level_trigger : public game_object {
private:
    std::shared_ptr<game_level> at_level = nullptr;
    std::wstring next_level_identifier;
    std::shared_ptr<game_level> next_level = nullptr;
    TRIGGER_TYPE trigger_type;
    float_pos land_pos;

public:
    level_trigger(const float_pos& place_pos, const float_pos& size,
        std::shared_ptr<game_level> at_level, const std::wstring& next_level_identifier,
        TRIGGER_TYPE trigger_type, const float_pos& land_pos);
    void update() override;
    void trigger() override { }
};

class collection_ironman : public game_object {
private:
    float_pos original_pos;

public:
    collection_ironman(const float_pos& place_pos, std::shared_ptr<game_level> at_level);
    void update() override;
    void trigger() override { }
    void draw_on(const float_pos& screen_pos);
};

class collection_weapon : public game_object {
private:
    float_pos original_pos;

public:
    collection_weapon(const float_pos& place_pos, std::shared_ptr<game_level> at_level);
    void update() override;
    void trigger() override { }
    void draw_on(const float_pos& screen_pos);
};