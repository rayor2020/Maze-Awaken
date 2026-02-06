#pragma once
#include "../utils/position.h"
#include "game_object.h"
#include "maze_router.h"
#include <easyx.h>
#include <fstream>
#include <list>
#include <memory>
#include <vector>
class GameMain;
using game_ptr = std::shared_ptr<GameMain>;
class game_level : public std::enable_shared_from_this<game_level> {
private:
    using obj_ptr = std::shared_ptr<game_object>;
    std::list<obj_ptr> obj_list; // 游戏对象列表
    std::list<obj_ptr> new_obj_list; // 新添加对象列表
    bool is_active; // 是否正在运行
    float_pos player_spawn_point; // 玩家重生点
    IMAGE background; // 背景
    std::wstring level_name; // 关卡名称
    std::wstring level_dat_path; // 关卡信息路径
    std::shared_ptr<player> holding_player; // 所操作的玩家
    game_ptr in_game; // 所处的游戏实例
    bool game_maze_map[MAZE_HEIGHT][MAZE_WIDTH]; // 关卡迷宫数据
    bool is_maze_level; // 是否为迷宫关
    maze_router router; // 寻路机
    // 预计算的屏幕中心偏移值，避免每帧重复计算
    float screen_center_x;
    float screen_center_y;

private:
    // 新的坐标转换方法，使用预计算值提高性能
    float_pos to_screen_coord(const float_pos& real_coord, const float_pos& camera_pos) const;
    static std::vector<std::wstring> split(const std::wstring& line, wchar_t space = L' ');
    void load_level_from(const std::wstring& level_dat);

public:
    game_level(const std::wstring& background_path, const std::wstring& level_dat, game_ptr game);
    // 根据名称获取关卡指针
    std::shared_ptr<game_level> get_level_from_name(const std::wstring&);
    // 开始关卡
    void start();
    // 重置关卡
    void reset();
    // 返回关卡运行状态
    bool active() const;
    // 关闭关卡
    void deactivate();
    // 添加对象
    void add_object(obj_ptr new_obj);
    // 在下一帧统一添加对象
    void add_object_request(obj_ptr new_obj);
    // 绘制帧
    void draw_level_frame(const float_pos& camera_pos) const;
    // 关卡出生点
    float_pos spawn_point() const;
    // 名称
    std::wstring name() const;
    // 帧更新
    void update();
    // 设置玩家指针
    void set_player(std::shared_ptr<player> p);
    // (向上级请求)切换关卡
    void change_level(std::shared_ptr<game_level> next_level, const float_pos& new_pos);
    // 转场动画
    void shift_animation(bool to_credit = false);
    // 是否为迷宫关卡
    bool is_maze() const;
    // 返回迷宫信息
    bool maze_map(const int_pos& p) const;
    // 返回所属的游戏实例
    game_ptr game_instance();
    // 返回所拥有的玩家实例
    std::shared_ptr<player> get_player();
    // 返回是否存在某种对象
    bool has(OBJECT_TYPE type);
    // 返回第一个敌人指针
    obj_ptr enemy();
    // 重启游戏
    void restart_game();
    // 碰撞检测
public:
    // 碰撞箱碰撞检测
    static bool hitbox_intersect(
        const float_pos& box1_ul, const float_pos& box1_dr,
        const float_pos& box2_ul, const float_pos& box2_dr);
    // 对象间碰撞检测
    bool collide(const obj_ptr& obj1, const obj_ptr& obj2);
    // 全局碰撞检测列表
    std::vector<obj_ptr> collide_with(const obj_ptr& from_obj);
    // 寻路列表
    std::queue<int_pos> route(const int_pos& p1, const int_pos& p2);
};