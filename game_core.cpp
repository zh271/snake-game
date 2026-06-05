#include "game_core.h"
#include "map.h"
#include "powerup.h"
#include <cstdlib>
#include <cstring>

// —— 内部辅助 ————————————————————————————————

static int g_snake1_move_timer = 0;
static int g_snake2_move_timer = 0;

// —— 方向工具 ————————————————————————————————

bool Direction_IsOpposite(Direction a, Direction b) {
    if (a == DIR_UP    && b == DIR_DOWN)  return true;
    if (a == DIR_DOWN  && b == DIR_UP)    return true;
    if (a == DIR_LEFT  && b == DIR_RIGHT) return true;
    if (a == DIR_RIGHT && b == DIR_LEFT)  return true;
    return false;
}

Point Position_Next(Point pos, Direction dir) {
    switch (dir) {
        case DIR_UP:    return Point(pos.x, pos.y - 1);
        case DIR_DOWN:  return Point(pos.x, pos.y + 1);
        case DIR_LEFT:  return Point(pos.x - 1, pos.y);
        case DIR_RIGHT: return Point(pos.x + 1, pos.y);
        default:        return pos;
    }
}

// —— 初始化 ————————————————————————————————

void Snake_Init(Snake* snake, Point start_pos, Direction dir) {
    snake->length   = INIT_SNAKE_LENGTH;
    snake->dir      = dir;
    snake->next_dir = dir;
    snake->score    = 0;
    snake->alive    = true;
    snake->magnet_food_left = 0;
    snake->drill_food_left  = 0;
    snake->slow_food_left   = 0;
    snake->move_interval_ms = NORMAL_MOVE_INTERVAL_MS;

    // 根据初始方向反向摆放蛇身
    for (int i = 0; i < INIT_SNAKE_LENGTH; i++) {
        switch (dir) {
            case DIR_UP:    snake->body[i] = Point(start_pos.x, start_pos.y + i); break;
            case DIR_DOWN:  snake->body[i] = Point(start_pos.x, start_pos.y - i); break;
            case DIR_LEFT:  snake->body[i] = Point(start_pos.x + i, start_pos.y); break;
            case DIR_RIGHT: snake->body[i] = Point(start_pos.x - i, start_pos.y); break;
            default: break;
        }
    }
}

void Game_Init(GameState* state, GameMode mode) {
    state->mode   = mode;
    state->status = GAME_RUNNING;
    state->step_count       = 0;
    state->random_event_step = 150; // N步后触发随机事件
    state->has_snake2       = (mode == MODE_VERSUS_PVP || mode == MODE_VERSUS_AI);
    state->snake2_is_ai     = (mode == MODE_VERSUS_AI);

    // 初始化地图
    Map_Init(&state->map);

    // 初始化蛇1 (左上区域)
    Snake_Init(&state->snake1, Point(5, 25), DIR_RIGHT);

    // 初始化蛇2 (右下区域)
    if (state->has_snake2) {
        Snake_Init(&state->snake2, Point(44, 25), DIR_LEFT);
    } else {
        // 重置蛇2为空状态
        memset(&state->snake2, 0, sizeof(Snake));
    }

    // 生成障碍物
    Map_GenerateObstacles(state, OBSTACLE_COUNT);

    // 生成食物
    Map_GenerateFood(state);

    // 道具初始化
    if (mode == MODE_POWERUP || mode == MODE_VERSUS_PVP || mode == MODE_VERSUS_AI) {
        PowerUp_Init(state);
    } else {
        for (int i = 0; i < POWERUP_ON_MAP; i++) {
            state->powerups[i].active = false;
            state->powerups[i].type   = POWER_NONE;
        }
    }

    g_snake1_move_timer = 0;
    g_snake2_move_timer = 0;
}

// —— 碰撞检测 ————————————————————————————————

bool Game_CheckWallCollision(Point pos) {
    return pos.x < 0 || pos.x >= MAP_WIDTH ||
           pos.y < 0 || pos.y >= MAP_HEIGHT;
}

bool Game_CheckObstacleCollision(GameState* state, Snake* snake, Point head_pos) {
    if (!Map_IsInside(head_pos)) return false;
    if (state->map.cells[head_pos.y][head_pos.x] != CELL_OBSTACLE) return false;
    // 有钻头效果则不死
    if (snake->drill_food_left > 0) return false;
    return true;
}

bool Snake_CheckSelfCollision(Snake* snake, Point head_pos) {
    // 从第1节开始检查（跳过蛇头自己）
    for (int i = 1; i < snake->length; i++) {
        if (snake->body[i] == head_pos) return true;
    }
    return false;
}

bool Snake_CheckOtherBodyCollision(Snake* attacker, Snake* target) {
    Point head = attacker->body[0];
    for (int i = 0; i < target->length; i++) {
        if (target->body[i] == head) return true;
    }
    return false;
}

bool Snake_CheckHeadCollision(Snake* a, Snake* b) {
    return a->body[0] == b->body[0];
}

// —— 蛇移动 ————————————————————————————————

void Snake_SetDirection(Snake* snake, Direction dir) {
    if (dir == DIR_NONE) return;
    // 禁止反向
    if (Direction_IsOpposite(snake->dir, dir)) return;
    snake->next_dir = dir;
}

void Snake_Move(GameState* state, Snake* snake) {
    if (!snake->alive) return;

    // 采纳新方向
    snake->dir = snake->next_dir;

    // 计算新蛇头
    Point new_head = Position_Next(snake->body[0], snake->dir);

    // 撞墙
    if (Game_CheckWallCollision(new_head)) {
        snake->alive = false;
        return;
    }

    // 撞障碍物
    if (Game_CheckObstacleCollision(state, snake, new_head)) {
        snake->alive = false;
        return;
    }

    // 撞自己
    if (Snake_CheckSelfCollision(snake, new_head)) {
        snake->alive = false;
        return;
    }

    // 对战碰撞（如果第二条蛇存在且存活）
    if (state->has_snake2 && state->snake2.alive) {
        Snake* other = (snake == &state->snake1) ? &state->snake2 : &state->snake1;

        // 头撞头 → 双死
        if (Snake_CheckHeadCollision(snake, other)) {
            snake->alive  = false;
            other->alive  = false;
            return;
        }

        // 头撞对方身体 → 自己死
        if (Snake_CheckOtherBodyCollision(snake, other)) {
            snake->alive = false;
            return;
        }
    }

    // —— 检测吃食物（在移动蛇身之前）——
    bool ate_food = (new_head == state->map.food_pos);

    // —— 移动蛇身 ——
    // 如果吃到食物，先增长长度再挪身体，这样尾巴不会被丢掉
    if (ate_food) {
        snake->length++;
    }
    for (int i = snake->length - 1; i > 0; i--) {
        snake->body[i] = snake->body[i - 1];
    }
    snake->body[0] = new_head;

    if (ate_food) {
        Game_HandleEatFood(state, snake);
    }

    // —— 磁铁吸附 ————————————————
    if (snake->magnet_food_left > 0) {
        PowerUp_HandleMagnet(state, snake);
    }

    // —— 检测吃道具 ————————————————
    if (state->mode == MODE_POWERUP || state->mode == MODE_VERSUS_PVP || state->mode == MODE_VERSUS_AI) {
        PowerUp_CheckEat(state, snake);
    }
}

// —— 吃食物 ————————————————————————————————

void Game_HandleEatFood(GameState* state, Snake* snake) {
    snake->score++;

    // 注意：蛇长已在 Snake_Move 中增长，此处不再 ++

    // 道具持续次数更新
    PowerUp_UpdateEffectAfterEating(snake);

    // 刷新食物
    Map_GenerateFood(state);

    // 道具维护
    if (state->mode == MODE_POWERUP || state->mode == MODE_VERSUS_PVP || state->mode == MODE_VERSUS_AI) {
        PowerUp_Maintain(state);
    }
}

// —— 每帧更新 ————————————————————————————————

void Game_Update(GameState* state, int delta_ms) {
    if (state->status != GAME_RUNNING) return;

    // 蛇1移动计时
    g_snake1_move_timer += delta_ms;
    if (g_snake1_move_timer >= state->snake1.move_interval_ms) {
        g_snake1_move_timer -= state->snake1.move_interval_ms;
        Snake_Move(state, &state->snake1);
        state->step_count++;
    }

    // 蛇2移动计时
    if (state->has_snake2 && state->snake2.alive) {
        g_snake2_move_timer += delta_ms;
        if (g_snake2_move_timer >= state->snake2.move_interval_ms) {
            g_snake2_move_timer -= state->snake2.move_interval_ms;
            Snake_Move(state, &state->snake2);
        }
    }

    // 随机事件：每 N 步随机增加障碍物
    if (state->step_count >= state->random_event_step) {
        state->step_count = 0;
        // 重新随机 N
        state->random_event_step = 100 + (rand() % 200);
        // 随机加几个障碍物
        int extra = 1 + (rand() % 3);
        Map_GenerateObstacles(state, extra);
    }

    Game_CheckGameOver(state);
}

// —— 游戏结束判定 ————————————————————————————————

void Game_CheckGameOver(GameState* state) {
    if (state->has_snake2) {
        // 对战模式：至少一条蛇死
        if (!state->snake1.alive || !state->snake2.alive) {
            state->status = GAME_OVER;
        }
    } else {
        // 单人模式
        if (!state->snake1.alive) {
            state->status = GAME_OVER;
        }
    }
}

int Game_GetWinner(GameState* state) {
    if (state->status != GAME_OVER) return 0;
    if (!state->has_snake2) return 1; // 单人模式，蛇1死

    bool s1 = state->snake1.alive;
    bool s2 = state->snake2.alive;

    if (!s1 && !s2) return 0;           // 双死平局
    if ( s1 &&  s2) {
        // 都比较分数
        if (state->snake1.score > state->snake2.score) return 1;
        if (state->snake2.score > state->snake1.score) return 2;
        return 0; // 平分平局
    }
    if (s1) return 1;
    return 2;
}
