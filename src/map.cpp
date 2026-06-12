#include "map.h"
#include <cstdlib>
#include <ctime>

// —— 随机数辅助 ————————————————————————————————

static int rand_range(int min, int max) {
    return min + rand() % (max - min + 1);
}

// —— Map ——————————————————————————————————————

void Map_Init(Map* map) {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            map->cells[y][x] = CELL_EMPTY;
        }
    }
    map->food_pos = Point(-1, -1);
}

void Map_SetCell(Map* map, Point pos, CellType type) {
    if (!Map_IsInside(pos)) return;
    map->cells[pos.y][pos.x] = type;
}

CellType Map_GetCell(Map* map, Point pos) {
    if (!Map_IsInside(pos)) return CELL_EMPTY;
    return map->cells[pos.y][pos.x];
}

bool Map_IsInside(Point pos) {
    return pos.x >= 0 && pos.x < MAP_WIDTH &&
           pos.y >= 0 && pos.y < MAP_HEIGHT;
}

bool Snake_ContainsPosition(Snake* snake, Point pos) {
    for (int i = 0; i < snake->length; i++) {
        if (snake->body[i] == pos) return true;
    }
    return false;
}

bool Map_IsOccupiedBySnake(GameState* state, Point pos) {
    if (Snake_ContainsPosition(&state->snake1, pos)) return true;
    if (state->has_snake2 && Snake_ContainsPosition(&state->snake2, pos)) return true;
    return false;
}

bool Map_IsCellEmpty(GameState* state, Point pos) {
    if (!Map_IsInside(pos)) return false;
    if (state->map.cells[pos.y][pos.x] != CELL_EMPTY) return false;
    if (Map_IsOccupiedBySnake(state, pos)) return false;
    if (pos == state->map.food_pos) return false;
    for (int i = 0; i < POWERUP_ON_MAP; i++) {
        if (state->powerups[i].active && state->powerups[i].pos == pos) return false;
    }
    return true;
}

Point Map_GetRandomEmptyCell(GameState* state) {
    // 收集所有空格子
    static Point empty_list[MAP_WIDTH * MAP_HEIGHT];
    int count = 0;
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            Point p(x, y);
            if (Map_IsCellEmpty(state, p)) {
                empty_list[count++] = p;
            }
        }
    }
    if (count == 0) return Point(0, 0); // 极端情况
    return empty_list[rand() % count];
}

// —— 障碍物 ————————————————————————————————————

void Map_GenerateObstacles(GameState* state, int count) {
    for (int i = 0; i < count; i++) {
        Point p = Map_GetRandomEmptyCell(state);
        if (p.x >= 0) {
            Map_SetCell(&state->map, p, CELL_OBSTACLE);
        }
    }
}

// —— 食物 ——————————————————————————————————————

void Map_GenerateFood(GameState* state) {
    // 先清除旧食物标记
    if (state->map.food_pos.x >= 0) {
        Map_SetCell(&state->map, state->map.food_pos, CELL_EMPTY);
    }
    Point p = Map_GetRandomEmptyCell(state);
    if (p.x >= 0) {
        state->map.food_pos = p;
        Map_SetCell(&state->map, p, CELL_FOOD);
    }
}
