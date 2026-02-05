#pragma once
#ifndef MAZEUTILITY_H
#define MAZEUTILITY_H
#include <easyx.h>
#include <string>
const int GAME_WINDOW_WIDTH = 1920;
const int GAME_WINDOW_HEIGHT = 1080;
const int GAME_BACKGROUND_TOTAL = 5;
enum class OBJECT_ID {
    // 游戏内的实体
    EMPTY = -1, // 空
    PLAYER = 0, // 玩家
    BOSS_MAZE_1, // Boss 1号
    BOSS_MAZE_2, // Boss 2号
    BOSS_MAZE_FINAL, // 最终Boss
    SHADOW_PLAYER, // 隐藏关卡的追逐者
    // 游戏内物体
    PLATFORM = 100, // 平台
    SPIKE, // 尖刺
    GATE, // 大门
    TRIGGER, // (隐藏) 关卡触发器
    // 游戏内收集品
    ULTIMATE_SABER = 200, // 寰宇支配之剑
    KEY, // 门钥匙
    SABER, // 普通剑
    GOLDEN_HEART, // 金色之心 (一命通关收集品)
    // 游戏需要
    MAZE_MASK = 300, // 迷宫的迷雾
    MAZE_ROOM, // 迷宫房间
};
enum class OBJECT_TYPE {
    NONE,
    PLAYER,
    PLATFORM,
    TRIGGER,
    SPIKE,
    LAVA,
    PARTICLE,
    ENEMY,
    BULLET,
    LAZER,
    COLLECTION
};
enum class MOVE_DIRECTION {
    STOP,
    RIGHT,
    LEFT,
    UP,
    DOWN,
    JUMP,
    DASH_UP,
    DASH_DOWN,
    DASH_RIGHT,
    DASH_LEFT,
    DASH_UP_LEFT,
    DASH_UP_RIGHT,
    DASH_DOWN_LEFT,
    DASH_DOWN_RIGHT
};
enum class GAME_STATUS {
    TITLESCREEN,
    GAMING,
    PAUSE,
    WAITING,
    END,
    CREDIT,
    RESTART
};
enum class TRIGGER_TYPE {
    BOUND, // 碰撞触发
    SPEED, // 速度触发(需要先碰撞)
    END, // 游戏结束
};
// 关卡总数
const int LEVEL_TOTAL = 15;
// 关卡标识符
const std::wstring level_id[LEVEL_TOTAL] = {
    L"level_1B",
    L"level_1C",
    L"level_2A",
    L"level_2B",
    L"level_2C",
    L"level_2D",
    L"level_3A",
    L"level_3D",
    L"level_4C",
    L"level_4D",
    L"level_5C",
    L"level_6C",
    L"level_6B",
    L"level_7B",
    L"level_end"
};
// 默认缺省路径
const std::string DEFAULT_ASSET_PATH = "assets\\textures\\default.png";
// 游戏运行的帧率
const int GAME_FPS = 60;
// 收集品总数
const int MAZE_COLLECTIONS_TOTAL = 10;
// Boss 血量
enum class BOSS_HEALTH {
    BOSS_1_HEALTH = 4,
    BOSS_2_HEALTH = 8,
    BOSS_FINAL_HEALTH = 20
};
// 玩家最大移动速度(px)
const double MAX_SPEED = 17.5;
// 玩家下落速度
const double MAX_FALLING_SPEED = 14.0;
// 玩家最大体力
const int MAX_STAMANIA = 200;
// 玩家冲刺速度
const int DASH_SPEED = 30;
// 玩家动画总数
const int PLAYER_ANIMATION_TOTAL = 2;
// 玩家步行动画帧数
const int PLAYER_WALK_FRAME = 8;
// 玩家站立动画帧数
const int PLAYER_STAND_FRAME = 6;
// 平台塌陷时间
const int PLATFORM_COLLAPSE_FRAME = 60;
// 虚拟迷宫宽度
const int MAZE_WIDTH = 32;
// 虚拟迷宫高度
const int MAZE_HEIGHT = 18;

#endif
// MAZEUTILITY_H
