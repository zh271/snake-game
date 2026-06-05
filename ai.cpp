#include "ai.h"
#include "game_core.h"
#include "map.h"
#include <cstdlib>
#include <cmath>

int AI_ManhattanDistance(Point a, Point b) {
    return abs(a.x - b.x) + abs(a.y - b.y);
}

bool AI_IsDirectionSafe(GameState* state, Snake* ai_snake, Snake* enemy_snake, Direction dir) {
    Point next = Position_Next(ai_snake->body[0], dir);

    // 越界
    if (!Map_IsInside(next)) return false;

    // 撞障碍（钻头效果不考虑，保守一点）
    if (state->map.cells[next.y][next.x] == CELL_OBSTACLE && ai_snake->drill_food_left <= 0)
        return false;

    // 撞自己
    for (int i = 1; i < ai_snake->length; i++) {
        if (ai_snake->body[i] == next) return false;
    }

    // 撞敌人身体（包括蛇头）
    if (enemy_snake && enemy_snake->alive) {
        for (int i = 0; i < enemy_snake->length; i++) {
            if (enemy_snake->body[i] == next) return false;
        }
    }

    return true;
}

int AI_GetSafeDirections(GameState* state, Snake* ai_snake, Snake* enemy_snake, Direction safe_dirs[4]) {
    Direction all[4] = { DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT };
    int count = 0;
    for (int i = 0; i < 4; i++) {
        if (AI_IsDirectionSafe(state, ai_snake, enemy_snake, all[i])) {
            safe_dirs[count++] = all[i];
        }
    }
    return count;
}

Direction AI_ChooseDirectionToFood(GameState* state, Snake* ai_snake, Direction safe_dirs[4], int safe_count) {
    if (safe_count == 0) return DIR_NONE;

    Point food = state->map.food_pos;
    Point head = ai_snake->body[0];

    Direction best = safe_dirs[0];
    int best_dist = 999999;

    for (int i = 0; i < safe_count; i++) {
        Point next = Position_Next(head, safe_dirs[i]);
        int dist = AI_ManhattanDistance(next, food);
        if (dist < best_dist) {
            best_dist = dist;
            best = safe_dirs[i];
        }
    }
    return best;
}

Direction AI_ChooseRandomSafeDirection(Direction safe_dirs[4], int safe_count) {
    if (safe_count == 0) return DIR_NONE;
    return safe_dirs[rand() % safe_count];
}

Direction AI_GetNextDirection(GameState* state, Snake* ai_snake, Snake* enemy_snake) {
    if (!ai_snake->alive) return DIR_NONE;

    Direction safe_dirs[4];
    int safe_count = AI_GetSafeDirections(state, ai_snake, enemy_snake, safe_dirs);

    if (safe_count == 0) {
        // 无处可走，保持当前方向（等死）
        return ai_snake->dir;
    }

    // 以 80% 概率朝食物走，20% 随机
    if (safe_count > 1 && (rand() % 100) < 20) {
        return AI_ChooseRandomSafeDirection(safe_dirs, safe_count);
    }

    return AI_ChooseDirectionToFood(state, ai_snake, safe_dirs, safe_count);
}
