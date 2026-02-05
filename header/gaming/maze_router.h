#pragma once
#include <vector>
#include <algorithm>
#include <numeric>
#include <queue>
#include <list>
#include "../utils/position.h"
#include "../utils/maze_utils.h"
class maze_router {
private:
    bool maze_map[MAZE_HEIGHT][MAZE_WIDTH];
    struct node {
        int_pos point;
        double f;
        bool operator<(const node& node2) const {
            return f > node2.f;
        }
    };
public:
    maze_router() {
        memset(maze_map, 0, sizeof(bool) * MAZE_WIDTH * MAZE_HEIGHT);
    }
    maze_router(const std::wstring& info) {
        for (int y = 0; y < MAZE_HEIGHT; ++y) {
            for (int x = 0; x < MAZE_WIDTH; ++x)
                maze_map[y][x] = info[y * MAZE_WIDTH + x] == L'1';
        }
    }
public:
    std::queue<int_pos> point_of_route(const int_pos& pos1, const int_pos& pos2) const{
        const int_pos delta_pos[4] = {
            {-1,0},{0,1},{1,0},{0,-1}
        };
        bool visited[MAZE_HEIGHT][MAZE_WIDTH] = { false };
        double value[MAZE_HEIGHT][MAZE_WIDTH] = { 0.0 };
        int_pos parent[MAZE_HEIGHT][MAZE_WIDTH];
        for (int y = 0; y < MAZE_HEIGHT; ++y) {
            for (int x = 0; x < MAZE_WIDTH; ++x) {
                visited[y][x] = false;
                value[y][x] = std::numeric_limits<double>::infinity();
                parent[y][x] = int_pos{ -1, -1 };
            }
        }
        auto evaluate = [&](const int_pos& a, const int_pos& b) {
            return abs(a.x - b.x) + abs(a.y - b.y);
        };
        std::priority_queue<node> list;
        value[pos1.y][pos1.x] = 0;
        list.push(node(pos1, evaluate(pos1, pos2)));
        while (!list.empty()) {
            node current = list.top();
            list.pop();
            int_pos point = current.point;
            if (visited[point.y][point.x])
                continue;
            visited[point.y][point.x] = true;
            if (point == pos2)
                break;
            for (int i = 0; i < 4; i++) {
                int_pos now_point = point.offset(delta_pos[i]);
                if (now_point.x < 0 || now_point.x >= MAZE_WIDTH || 
                    now_point.y < 0 || now_point.y >= MAZE_HEIGHT)
                    continue;
                if (!maze_map[now_point.y][now_point.x])
                    continue;
                double now_value = value[point.y][point.x] + 1.0f;
                if (now_value < value[now_point.y][now_point.x]) {
                    value[now_point.y][now_point.x] = now_value;
                    parent[now_point.y][now_point.x] = point;
                    list.push({
                        int_pos{ now_point.x, now_point.y },
                        now_value + evaluate(int_pos{ now_point.x, now_point.y }, pos2)
                    });
                }
            }
        }
        std::vector<int_pos> path;
        if (!visited[pos2.y][pos2.x])
            return {};
        int_pos now_point = pos2;
        while (now_point != int_pos(-1, -1)) {
            path.push_back(now_point);
            if (now_point == pos1)
                break;
            now_point = parent[now_point.y][now_point.x];
        }
        std::reverse(path.begin(), path.end());
        std::queue<int_pos> result_path;
        for (auto& p : path)
            result_path.push(p);
        return result_path;
    }
};