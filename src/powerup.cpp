#include "powerup.h"
#include "map.h"
#include "snake_music.h"
#include <cstdlib>
#include <cmath>

// ============================================================
// PowerUpManager 成员函数实现
// ============================================================

PowerUpManager::PowerUpManager() {
    // 随机种子由 main 负责设置
}

Position PowerUpManager::getRandomEmptyCell(GameState& state) {
    return Map_GetRandomEmptyCell(&state);
}

PowerUpType PowerUpManager::getRandomType() {
    int r = rand() % 5;
    switch (r) {
        case 0: return POWER_BIG_FOOD;
        case 1: return POWER_MAGNET;
        case 2: return POWER_DRILL;
        case 3: return POWER_DIGEST;
        case 4: return POWER_SLOW;
        default: return POWER_BIG_FOOD;
    }
}

bool PowerUpManager::isInMagnetRange(Position head, Position food) {
    return abs(head.x - food.x) <= 1 && abs(head.y - food.y) <= 1;
}

void PowerUpManager::init(GameState& state) {
    for (int i = 0; i < POWERUP_ON_MAP; i++) {
        spawn(state, i);
    }
}

void PowerUpManager::maintain(GameState& state) {
    for (int i = 0; i < POWERUP_ON_MAP; i++) {
        if (!state.powerups[i].active) {
            spawn(state, i);
        }
    }
}

void PowerUpManager::spawn(GameState& state, int index) {
    Position pos = getRandomEmptyCell(state);
    if (pos.x < 0) return; // 无空格

    state.powerups[index].type   = getRandomType();
    state.powerups[index].pos    = pos;
    state.powerups[index].active = true;
    Map_SetCell(&state.map, pos, CELL_POWERUP);
}

bool PowerUpManager::checkEat(GameState& state, Snake& snake) {
    Point head = snake.body[0];
    for (int i = 0; i < POWERUP_ON_MAP; i++) {
        if (!state.powerups[i].active) continue;
        if (state.powerups[i].pos == head) {
            PowerUpType type = state.powerups[i].type;
            state.powerups[i].active = false;
            state.powerups[i].type   = POWER_NONE;
            // 清除地图标记
            Map_SetCell(&state.map, state.powerups[i].pos, CELL_EMPTY);
            apply(state, snake, type);
            return true;
        }
    }
    return false;
}

void PowerUpManager::apply(GameState& state, Snake& snake, PowerUpType type) {
    switch (type) {
        case POWER_BIG_FOOD:
            GameAudio_play(SOUND_EAT_BIG_FOOD);
            applyBigFood(state, snake);
            break;
        case POWER_MAGNET:
            GameAudio_play(SOUND_EAT_MAGNET);
            applyMagnet(snake);
            break;
        case POWER_DRILL:
            GameAudio_play(SOUND_EAT_DRILL);
            applyDrill(snake);
            break;
        case POWER_DIGEST:
            GameAudio_play(SOUND_EAT_DIGEST);
            applyDigest(snake);
            break;
        case POWER_SLOW:
            GameAudio_play(SOUND_EAT_SLOW);
            applySlow(snake);
            break;
        default: break;
    }
}

void PowerUpManager::applyBigFood(GameState& state, Snake& snake) {
    snake.score += 10;
    snake.length += 1;
}

void PowerUpManager::applyMagnet(Snake& snake) {
    snake.magnet_food_left = 2;
}

void PowerUpManager::applyDrill(Snake& snake) {
    snake.drill_food_left = 2;
}

void PowerUpManager::applyDigest(Snake& snake) {
    // 分数不变，长度变为 2/3，最少保留 3 节
    int new_len = snake.length * 2 / 3;
    if (new_len < INIT_SNAKE_LENGTH) new_len = INIT_SNAKE_LENGTH;
    snake.length = new_len;
}

void PowerUpManager::applySlow(Snake& snake) {
    snake.slow_food_left = 2;
    snake.move_interval_ms = SLOW_MOVE_INTERVAL_MS;
}

void PowerUpManager::updateEffectAfterEating(Snake& snake) {
    if (snake.magnet_food_left > 0) {
        snake.magnet_food_left--;
    }
    if (snake.drill_food_left > 0) {
        snake.drill_food_left--;
    }
    if (snake.slow_food_left > 0) {
        snake.slow_food_left--;
        if (snake.slow_food_left == 0) {
            snake.move_interval_ms = NORMAL_MOVE_INTERVAL_MS;
        }
    }
}

void PowerUpManager::handleMagnet(GameState& state, Snake& snake) {
    Point head = snake.body[0];
    Point& food = state.map.food_pos;

    if (food.x < 0) return; // 无食物

    if (isInMagnetRange(head, food)) {
        // 吸附食物
        snake.score++;
        snake.length++;
        GameAudio_play(SOUND_EAT_FOOD);
        snake.magnet_food_left--;
        if (snake.magnet_food_left < 0) snake.magnet_food_left = 0;

        // 刷新食物
        Map_GenerateFood(&state);

        // 道具维护
        maintain(state);
    }
}

// ============================================================
// 全局单例 + 自由函数接口（供 game_core 调用）
// ============================================================

static PowerUpManager g_powerup_mgr;

void PowerUp_Init(GameState* state) {
    g_powerup_mgr.init(*state);
}

void PowerUp_Maintain(GameState* state) {
    g_powerup_mgr.maintain(*state);
}

void PowerUp_Spawn(GameState* state, int index) {
    g_powerup_mgr.spawn(*state, index);
}

bool PowerUp_CheckEat(GameState* state, Snake* snake) {
    return g_powerup_mgr.checkEat(*state, *snake);
}

void PowerUp_Apply(GameState* state, Snake* snake, PowerUpType type) {
    g_powerup_mgr.apply(*state, *snake, type);
}

void PowerUp_UpdateEffectAfterEating(Snake* snake) {
    g_powerup_mgr.updateEffectAfterEating(*snake);
}

void PowerUp_HandleMagnet(GameState* state, Snake* snake) {
    g_powerup_mgr.handleMagnet(*state, *snake);
}
