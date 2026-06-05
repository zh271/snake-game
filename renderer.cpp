#include "renderer.h"
#include "game_core.h"
#include "raylib.h"
#include <cmath>
#include <cstdio>
#include <ctime>
#include <cstdlib>

// ============================================================
// 全局状态 & 颜色主题
// ============================================================
static float g_anim_time = 0.0f;

// —— 调色板：深色赛博朋克风 —————
static const Color BG_DARK      = { 15, 17, 30, 255 };
static const Color BG_MID       = { 22, 24, 42, 255 };
static const Color GRID_LINE    = { 35, 38, 60, 255 };
static const Color GRID_BG      = { 18, 20, 35, 255 };
static const Color BORDER_GLOW  = { 60, 120, 255, 40 };

static const Color SNAKE1_HEAD  = { 80, 220, 100, 255 };
static const Color SNAKE1_BODY  = { 40, 180, 60, 255 };
static const Color SNAKE1_TAIL  = { 20, 100, 30, 255 };
static const Color SNAKE1_GLOW  = { 80, 255, 100, 60 };

static const Color SNAKE2_HEAD  = { 255, 160, 40, 255 };
static const Color SNAKE2_BODY  = { 220, 120, 20, 255 };
static const Color SNAKE2_TAIL  = { 160, 70, 10, 255 };
static const Color SNAKE2_GLOW  = { 255, 180, 50, 60 };

static const Color FOOD_COLOR   = { 255, 70, 70, 255 };
static const Color FOOD_GLOW    = { 255, 80, 80, 100 };
static const Color OBSTACLE_CLR = { 80, 80, 100, 255 };
static const Color OBSTACLE_TOP = { 100, 100, 120, 255 };

static const Color PANEL_BG     = { 25, 28, 50, 230 };
static const Color PANEL_BORDER = { 50, 55, 90, 255 };
static const Color TEXT_PRIMARY = { 220, 225, 255, 255 };
static const Color TEXT_ACCENT  = { 100, 200, 255, 255 };
static const Color TEXT_GOLD    = { 255, 210, 80, 255 };
static const Color TEXT_DIM     = { 130, 135, 160, 255 };

// 道具颜色
static const Color POWER_BIG_FOOD_CLR = { 255, 80, 180, 255 };
static const Color POWER_MAGNET_CLR   = { 180, 80, 255, 255 };
static const Color POWER_DRILL_CLR    = { 255, 200, 50, 255 };
static const Color POWER_DIGEST_CLR   = { 50, 220, 200, 255 };
static const Color POWER_SLOW_CLR     = { 130, 180, 255, 255 };

// ============================================================
// 工具函数
// ============================================================
static Color LerpColor(Color a, Color b, float t) {
    return Color{
        (unsigned char)(a.r + (b.r - a.r) * t),
        (unsigned char)(a.g + (b.g - a.g) * t),
        (unsigned char)(a.b + (b.b - a.b) * t),
        255
    };
}

static void DrawRoundRect(int x, int y, int w, int h, float r, Color c) {
    DrawRectangleRounded({(float)x, (float)y, (float)w, (float)h}, r, 8, c);
}

// 将地图坐标转为屏幕像素坐标
static int GridToScreenX(int gx) { return GRID_OFFSET_X + gx * CELL_SIZE; }
static int GridToScreenY(int gy) { return GRID_OFFSET_Y + gy * CELL_SIZE; }

// ============================================================
// 生命周期
// ============================================================
void Renderer_Init() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Snake Battle — Exquisite Edition");
    SetTargetFPS(60);
    SetExitKey(0); // 手动处理退出
    srand((unsigned)time(nullptr));
}

void Renderer_Clear() {
    BeginDrawing();
    ClearBackground(BG_DARK);
}

void Renderer_Present() {
    EndDrawing();
    g_anim_time += GetFrameTime();
}

void Renderer_Destroy() {
    CloseWindow();
}

float Renderer_GetFrameTime() {
    return GetFrameTime();
}

// ============================================================
// 网格
// ============================================================
void Renderer_DrawGrid() {
    int grid_w = MAP_WIDTH * CELL_SIZE;
    int grid_h = MAP_HEIGHT * CELL_SIZE;

    // 网格背景
    DrawRectangle(GRID_OFFSET_X - 2, GRID_OFFSET_Y - 2,
                  grid_w + 4, grid_h + 4, GRID_BG);

    // 细线网格
    for (int i = 0; i <= MAP_WIDTH; i++) {
        int x = GRID_OFFSET_X + i * CELL_SIZE;
        DrawLine(x, GRID_OFFSET_Y, x, GRID_OFFSET_Y + grid_h, GRID_LINE);
    }
    for (int i = 0; i <= MAP_HEIGHT; i++) {
        int y = GRID_OFFSET_Y + i * CELL_SIZE;
        DrawLine(GRID_OFFSET_X, y, GRID_OFFSET_X + grid_w, y, GRID_LINE);
    }

    // 外框发光
    DrawRectangleLinesEx(
        {(float)GRID_OFFSET_X - 3, (float)GRID_OFFSET_Y - 3,
         (float)grid_w + 6, (float)grid_h + 6},
        2.0f, BORDER_GLOW);
    DrawRectangleLinesEx(
        {(float)GRID_OFFSET_X - 1, (float)GRID_OFFSET_Y - 1,
         (float)grid_w + 2, (float)grid_h + 2},
        1.0f, {80, 85, 120, 255});
}

// ============================================================
// 障碍物
// ============================================================
void Renderer_DrawObstacles(Map* map) {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            if (map->cells[y][x] != CELL_OBSTACLE) continue;
            int sx = GridToScreenX(x);
            int sy = GridToScreenY(y);
            int cs = CELL_SIZE;

            // 主体
            DrawRectangle(sx + 1, sy + 1, cs - 2, cs - 2, OBSTACLE_CLR);
            // 顶面高光
            DrawRectangle(sx + 1, sy + 1, cs - 2, 3, OBSTACLE_TOP);
            // 内边框
            DrawRectangleLines(sx + 1, sy + 1, cs - 2, cs - 2,
                               {60, 60, 80, 255});
        }
    }
}

// ============================================================
// 食物（带脉冲动画 + 光晕）
// ============================================================
void Renderer_DrawFood(Map* map, float anim_t) {
    Point& food = map->food_pos;
    if (food.x < 0 || food.y < 0) return;

    int cx = GridToScreenX(food.x) + CELL_SIZE / 2;
    int cy = GridToScreenY(food.y) + CELL_SIZE / 2;
    float pulse = 1.0f + 0.2f * sinf(anim_t * 4.0f);
    float r = CELL_SIZE * 0.40f * pulse;

    // 外光晕
    for (int i = 3; i >= 0; i--) {
        float alpha = FOOD_GLOW.a * (1.0f - i * 0.25f);
        DrawCircle(cx, cy, r + i * 2.5f,
                   {FOOD_GLOW.r, FOOD_GLOW.g, FOOD_GLOW.b, (unsigned char)alpha});
    }

    // 主体红色圆
    DrawCircle(cx, cy, r, FOOD_COLOR);
    // 高光点
    DrawCircle(cx - r * 0.3f, cy - r * 0.3f, r * 0.35f,
               {255, 180, 180, 200});
}

// ============================================================
// 道具（独立图标 + 粒子光晕）
// ============================================================
static Color GetPowerUpColor(PowerUpType t) {
    switch (t) {
        case POWER_BIG_FOOD: return POWER_BIG_FOOD_CLR;
        case POWER_MAGNET:   return POWER_MAGNET_CLR;
        case POWER_DRILL:    return POWER_DRILL_CLR;
        case POWER_DIGEST:   return POWER_DIGEST_CLR;
        case POWER_SLOW:     return POWER_SLOW_CLR;
        default:             return WHITE;
    }
}

static const char* GetPowerUpLabel(PowerUpType t) {
    switch (t) {
        case POWER_BIG_FOOD: return "10";
        case POWER_MAGNET:   return "M";
        case POWER_DRILL:    return "D";
        case POWER_DIGEST:   return "-";
        case POWER_SLOW:     return "S";
        default:             return "?";
    }
}

void Renderer_DrawPowerUps(GameState* state, float anim_t) {
    for (int i = 0; i < POWERUP_ON_MAP; i++) {
        if (!state->powerups[i].active) continue;
        PowerUp& pu = state->powerups[i];
        int cx = GridToScreenX(pu.pos.x) + CELL_SIZE / 2;
        int cy = GridToScreenY(pu.pos.y) + CELL_SIZE / 2;
        float pulse = 1.0f + 0.15f * sinf(anim_t * 5.0f + i * 2.0f);
        float r = CELL_SIZE * 0.42f * pulse;
        Color c = GetPowerUpColor(pu.type);

        // 光晕
        for (int j = 2; j >= 0; j--) {
            DrawCircle(cx, cy, r + j * 3.0f,
                       {c.r, c.g, c.b, (unsigned char)(60 - j * 20)});
        }

        // 菱形主体
        DrawRectangle(cx - r, cy - r, r * 2, r * 2, c);
        DrawRectangleLinesEx({(float)(cx - r), (float)(cy - r), r * 2, r * 2},
                              1.5f, {255, 255, 255, 180});

        // 标签
        const char* label = GetPowerUpLabel(pu.type);
        int fs = (label[0] == '1') ? 12 : 14;
        int tw = MeasureText(label, fs);
        DrawText(label, cx - tw / 2, cy - fs / 2, fs, WHITE);
    }
}

// ============================================================
// 蛇绘制 — 渐变色 + 圆角段 + 光晕 + 眼睛
// ============================================================
static void DrawSnakeSegment(int gx, int gy, float t, int player_id,
                             bool is_head, bool is_tail, Direction dir) {
    int sx = GridToScreenX(gx);
    int sy = GridToScreenY(gy);
    int cs = CELL_SIZE;
    float pad = 1.2f;
    float rr = 0.35f;

    Color head_c, body_c, tail_c, glow_c;
    if (player_id == 1) {
        head_c = SNAKE1_HEAD; body_c = SNAKE1_BODY;
        tail_c = SNAKE1_TAIL; glow_c = SNAKE1_GLOW;
    } else {
        head_c = SNAKE2_HEAD; body_c = SNAKE2_BODY;
        tail_c = SNAKE2_TAIL; glow_c = SNAKE2_GLOW;
    }

    // 颜色插值
    Color seg_c;
    if (is_head) {
        seg_c = head_c;
    } else if (is_tail) {
        seg_c = LerpColor(body_c, tail_c, 0.6f);
    } else {
        seg_c = body_c;
    }

    // 外发光
    DrawRoundRect(sx - 1, sy - 1, cs - 1, cs - 1, rr + 0.1f, glow_c);

    // 主体
    DrawRoundRect(sx + pad, sy + pad, cs - pad * 2, cs - pad * 2, rr, seg_c);

    // 高光边
    Color hl = LerpColor(seg_c, WHITE, 0.25f);
    DrawRectangle(sx + pad + 1, sy + pad + 1, cs - pad * 2 - 2, 2, hl);

    // 蛇头：绘制眼睛
    if (is_head) {
        int cx = sx + cs / 2;
        int cy = sy + cs / 2;
        int eye_r = cs / 7;
        int eye_off = cs / 5;

        int ex1 = cx, ey1 = cy, ex2 = cx, ey2 = cy;
        switch (dir) {
            case DIR_UP:    ex1 = cx - eye_off; ey1 = cy - eye_off;
                           ex2 = cx + eye_off; ey2 = cy - eye_off; break;
            case DIR_DOWN:  ex1 = cx - eye_off; ey1 = cy + eye_off;
                           ex2 = cx + eye_off; ey2 = cy + eye_off; break;
            case DIR_LEFT:  ex1 = cx - eye_off; ey1 = cy - eye_off;
                           ex2 = cx - eye_off; ey2 = cy + eye_off; break;
            case DIR_RIGHT: ex1 = cx + eye_off; ey1 = cy - eye_off;
                           ex2 = cx + eye_off; ey2 = cy + eye_off; break;
            default: break;
        }
        DrawCircle(ex1, ey1, eye_r, WHITE);
        DrawCircle(ex2, ey2, eye_r, WHITE);
        DrawCircle(ex1, ey1, eye_r * 0.55f, {20, 20, 30, 255});
        DrawCircle(ex2, ey2, eye_r * 0.55f, {20, 20, 30, 255});
    }
}

void Renderer_DrawSnake(Snake* snake, int player_id, float anim_t) {
    if (!snake->alive) return;
    for (int i = snake->length - 1; i >= 0; i--) {
        bool is_head = (i == 0);
        bool is_tail = (i == snake->length - 1);
        Direction d = DIR_NONE;
        if (i == 0) d = snake->dir;
        else {
            Point& prev = snake->body[i - 1];
            Point& cur  = snake->body[i];
            if (prev.x < cur.x) d = DIR_LEFT;
            if (prev.x > cur.x) d = DIR_RIGHT;
            if (prev.y < cur.y) d = DIR_UP;
            if (prev.y > cur.y) d = DIR_DOWN;
        }
        DrawSnakeSegment(snake->body[i].x, snake->body[i].y,
                         (float)i / snake->length, player_id,
                         is_head, is_tail, d);
    }
}

// ============================================================
// 侧边栏 UI — 玻璃拟态面板
// ============================================================
void Renderer_DrawUI(GameState* state) {
    // 面板背景（半透明玻璃拟态）
    DrawRectangle(PANEL_X, 0, PANEL_WIDTH, WINDOW_HEIGHT, PANEL_BG);
    DrawLine(PANEL_X, 0, PANEL_X, WINDOW_HEIGHT, PANEL_BORDER);
    DrawLine(PANEL_X - 1, 0, PANEL_X - 1, WINDOW_HEIGHT,
             {30, 33, 58, 255});

    int tx = PANEL_X + 16;
    int ty = 30;

    // 标题
    DrawText("SNAKE", tx, ty, 28, TEXT_ACCENT);
    DrawText("BATTLE", tx, ty + 28, 20, TEXT_PRIMARY);
    ty += 70;

    // 分隔线
    DrawLine(tx, ty, PANEL_X + PANEL_WIDTH - 16, ty, PANEL_BORDER);
    ty += 15;

    // 模式
    const char* mode_str = "BASIC";
    if (state->mode == MODE_POWERUP)   mode_str = "POWER-UP";
    if (state->mode == MODE_VERSUS_PVP) mode_str = "PvP";
    if (state->mode == MODE_VERSUS_AI)  mode_str = "VS AI";
    DrawText("MODE", tx, ty, 12, TEXT_DIM);
    DrawText(mode_str, tx, ty + 14, 18, TEXT_ACCENT);
    ty += 45;

    // 分数 — 玩家1
    DrawText("PLAYER 1", tx, ty, 12, TEXT_DIM);
    char sbuf[32];
    snprintf(sbuf, sizeof(sbuf), "%d", state->snake1.score);
    int sw = MeasureText(sbuf, 36);
    DrawText(sbuf, PANEL_X + PANEL_WIDTH - 20 - sw, ty + 8, 36, SNAKE1_HEAD);
    // 长度
    snprintf(sbuf, sizeof(sbuf), "len: %d", state->snake1.length);
    DrawText(sbuf, tx, ty + 48, 12, TEXT_DIM);
    // 存活指示灯
    DrawCircle(tx + 6, ty + 68, 5, state->snake1.alive ? SNAKE1_HEAD : RED);
    DrawText(state->snake1.alive ? "ALIVE" : "DEAD", tx + 16, ty + 62, 13,
             state->snake1.alive ? SNAKE1_HEAD : RED);
    ty += 85;

    // 玩家2（如果存在）
    if (state->has_snake2) {
        DrawLine(tx, ty, PANEL_X + PANEL_WIDTH - 16, ty,
                 {50, 55, 90, 100});
        ty += 12;

        const char* p2_label = state->snake2_is_ai ? "AI" : "PLAYER 2";
        DrawText(p2_label, tx, ty, 12, TEXT_DIM);
        snprintf(sbuf, sizeof(sbuf), "%d", state->snake2.score);
        sw = MeasureText(sbuf, 36);
        DrawText(sbuf, PANEL_X + PANEL_WIDTH - 20 - sw, ty + 8, 36, SNAKE2_HEAD);
        snprintf(sbuf, sizeof(sbuf), "len: %d", state->snake2.length);
        DrawText(sbuf, tx, ty + 48, 12, TEXT_DIM);
        DrawCircle(tx + 6, ty + 68, 5, state->snake2.alive ? SNAKE2_HEAD : RED);
        DrawText(state->snake2.alive ? "ALIVE" : "DEAD", tx + 16, ty + 62, 13,
                 state->snake2.alive ? SNAKE2_HEAD : RED);
        ty += 85;
    }

    // 步数
    DrawLine(tx, ty, PANEL_X + PANEL_WIDTH - 16, ty,
             {50, 55, 90, 100});
    ty += 15;
    snprintf(sbuf, sizeof(sbuf), "STEPS: %d", state->step_count);
    DrawText(sbuf, tx, ty, 13, TEXT_DIM);
    ty += 30;

    // 道具状态
    Snake& s = state->snake1;
    auto drawEffect = [&](const char* name, int val, Color c, int& ypos) {
        if (val > 0) {
            char b[32];
            snprintf(b, sizeof(b), "%s: %d", name, val);
            DrawText(b, tx, ypos, 13, c);
            ypos += 20;
        }
    };
    drawEffect("MAGNET", s.magnet_food_left, POWER_MAGNET_CLR, ty);
    drawEffect("DRILL",  s.drill_food_left,  POWER_DRILL_CLR, ty);
    drawEffect("SLOW",   s.slow_food_left,   POWER_SLOW_CLR, ty);
    if (state->has_snake2) {
        drawEffect("MAGNET2", state->snake2.magnet_food_left, POWER_MAGNET_CLR, ty);
        drawEffect("DRILL2",  state->snake2.drill_food_left,  POWER_DRILL_CLR, ty);
        drawEffect("SLOW2",   state->snake2.slow_food_left,   POWER_SLOW_CLR, ty);
    }

    // 底部操作提示
    int by = WINDOW_HEIGHT - 100;
    DrawLine(tx, by, PANEL_X + PANEL_WIDTH - 16, by,
             {50, 55, 90, 100});
    by += 10;
    DrawText("WASD / Arrows", tx, by, 12, TEXT_DIM);
    DrawText("P: Pause", tx, by + 16, 12, TEXT_DIM);
    DrawText("Q: Quit", tx, by + 32, 12, TEXT_DIM);
    DrawText("ESC: Menu", tx, by + 48, 12, TEXT_DIM);
}

// ============================================================
// 主绘制入口
// ============================================================
void Renderer_DrawGame(GameState* state) {
    Renderer_DrawGrid();
    Renderer_DrawObstacles(&state->map);
    Renderer_DrawFood(&state->map, g_anim_time);
    Renderer_DrawPowerUps(state, g_anim_time);
    Renderer_DrawSnake(&state->snake1, 1, g_anim_time);
    if (state->has_snake2) {
        Renderer_DrawSnake(&state->snake2, 2, g_anim_time);
    }
    Renderer_DrawUI(state);
}

// ============================================================
// 游戏结束界面
// ============================================================
void Renderer_DrawGameOver(GameState* state) {
    // 暗色遮罩
    DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
                  {0, 0, 0, 200});

    // 居中面板
    int pw = 420, ph = 280;
    int px = (WINDOW_WIDTH - pw) / 2;
    int py = (WINDOW_HEIGHT - ph) / 2;

    // 面板玻璃效果
    DrawRectangle(px, py, pw, ph, {25, 28, 50, 245});
    DrawRectangleLinesEx({(float)px, (float)py, (float)pw, (float)ph},
                         2.0f, PANEL_BORDER);

    // "GAME OVER" 标题
    const char* title = "GAME OVER";
    int tw = MeasureText(title, 42);
    DrawText(title, (WINDOW_WIDTH - tw) / 2, py + 30, 42, TEXT_ACCENT);

    // 结果文字
    int winner = Game_GetWinner(state);
    const char* result;
    Color rc;
    if (!state->has_snake2) {
        result = "Better luck next time!";
        rc = TEXT_PRIMARY;
    } else if (winner == 0) {
        result = "IT'S A DRAW!";
        rc = TEXT_GOLD;
    } else if (winner == 1) {
        result = "PLAYER 1 WINS!";
        rc = SNAKE1_HEAD;
    } else {
        result = state->snake2_is_ai ? "AI WINS!" : "PLAYER 2 WINS!";
        rc = SNAKE2_HEAD;
    }
    tw = MeasureText(result, 28);
    DrawText(result, (WINDOW_WIDTH - tw) / 2, py + 90, 28, rc);

    // 分数
    char sbuf[64];
    snprintf(sbuf, sizeof(sbuf), "P1: %d pts", state->snake1.score);
    tw = MeasureText(sbuf, 20);
    DrawText(sbuf, (WINDOW_WIDTH - tw) / 2, py + 140, 20, SNAKE1_BODY);

    if (state->has_snake2) {
        snprintf(sbuf, sizeof(sbuf), "%s: %d pts",
                 state->snake2_is_ai ? "AI" : "P2", state->snake2.score);
        tw = MeasureText(sbuf, 20);
        DrawText(sbuf, (WINDOW_WIDTH - tw) / 2, py + 168, 20, SNAKE2_BODY);
    }

    // 操作提示
    const char* hint = "Press ENTER to return to menu";
    tw = MeasureText(hint, 16);
    DrawText(hint, (WINDOW_WIDTH - tw) / 2, py + ph - 40, 16, TEXT_DIM);
}

// ============================================================
// 主菜单 — 精美的模式选择
// ============================================================
int Renderer_DrawMenu() {
    // 背景
    ClearBackground(BG_DARK);

    // 装饰性背景网格
    for (int i = 0; i < WINDOW_WIDTH; i += 40) {
        DrawLine(i, 0, i, WINDOW_HEIGHT, {25, 28, 45, 255});
    }
    for (int i = 0; i < WINDOW_HEIGHT; i += 40) {
        DrawLine(0, i, WINDOW_WIDTH, i, {25, 28, 45, 255});
    }

    // 标题区域
    int ty = 80;
    const char* title = "SNAKE BATTLE";
    int tw = MeasureText(title, 56);
    DrawText(title, (WINDOW_WIDTH - tw) / 2, ty, 56, TEXT_ACCENT);

    const char* sub = "EXQUISITE EDITION";
    tw = MeasureText(sub, 18);
    DrawText(sub, (WINDOW_WIDTH - tw) / 2, ty + 60, 18, TEXT_DIM);

    // 装饰蛇动画
    float snaky = ty + 30 + sinf(g_anim_time * 2.0f) * 8.0f;
    for (int i = 0; i < 12; i++) {
        int sx = WINDOW_WIDTH / 2 - 180 + i * 30;
        float t = (float)i / 11.0f;
        Color sc = LerpColor(SNAKE1_HEAD, SNAKE1_TAIL, t);
        DrawCircle(sx, (int)snaky, 6, sc);
    }

    // 菜单项
    struct MenuItem {
        const char* name;
        const char* desc;
        Color color;
    };
    MenuItem items[] = {
        {"BASIC",       "Classic snake, obstacles, pure skill",        {80, 220, 100, 255}},
        {"POWER-UP",    "Items galore: magnet, drill, digest & more",  {180, 80, 255, 255}},
        {"PvP BATTLE",  "Two players, one keyboard, endless fun",      {255, 160, 40, 255}},
        {"VS AI",       "Face the machine — can you outsmart it?",     {255, 80, 80, 255}},
    };

    static int selection = 0;
    int start_y = 260;
    int item_h = 85;
    int item_w = 500;

    // 键盘导航
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        selection = (selection - 1 + 4) % 4;
    }
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        selection = (selection + 1) % 4;
    }

    for (int i = 0; i < 4; i++) {
        int ix = (WINDOW_WIDTH - item_w) / 2;
        int iy = start_y + i * (item_h + 8);
        bool sel = (i == selection);

        // 选中高亮
        Color bg = sel ? Color{40, 45, 75, 255} : Color{25, 28, 48, 230};
        DrawRoundRect(ix, iy, item_w, item_h, 12, bg);

        Color border = sel ? items[i].color : Color{50, 55, 90, 200};
        DrawRectangleLinesEx({(float)ix, (float)iy, (float)item_w, (float)item_h},
                              2.0f, border);

        if (sel) {
            // 左侧选中指示条
            DrawRectangle(ix - 6, iy + 6, 4, item_h - 12, items[i].color);
            DrawRectangle(ix - 6, iy + 6, 4, item_h - 12,
                          {items[i].color.r, items[i].color.g, items[i].color.b, 60});
        }

        // 选项名
        DrawText(items[i].name, ix + 24, iy + 12, 26, items[i].color);
        // 描述
        DrawText(items[i].desc, ix + 24, iy + 48, 13, TEXT_DIM);
    }

    // 底部提示
    const char* hint = "UP/DOWN to choose  |  ENTER to start  |  ESC to quit";
    tw = MeasureText(hint, 14);
    DrawText(hint, (WINDOW_WIDTH - tw) / 2, WINDOW_HEIGHT - 60, 14, TEXT_DIM);

    // 返回选择
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        switch (selection) {
            case 0: return MODE_BASIC;
            case 1: return MODE_POWERUP;
            case 2: return MODE_VERSUS_PVP;
            case 3: return MODE_VERSUS_AI;
        }
    }
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_Q)) {
        return -1; // 退出
    }
    return -2; // 仍在菜单
}

// ============================================================
// 暂停弹窗
// ============================================================
int Renderer_DrawPausePopup() {
    static int sel = 0;

    // 遮罩
    DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, {0, 0, 0, 180});

    int pw = 320, ph = 200;
    int px = (WINDOW_WIDTH - pw) / 2;
    int py = (WINDOW_HEIGHT - ph) / 2;

    DrawRoundRect(px, py, pw, ph, 14, {22, 25, 48, 250});
    DrawRectangleLinesEx({(float)px, (float)py, (float)pw, (float)ph}, 2, PANEL_BORDER);

    // 标题
    const char* title = "GAME PAUSED";
    int tw = MeasureText(title, 26);
    DrawText(title, (WINDOW_WIDTH - tw) / 2, py + 20, 26, TEXT_ACCENT);

    // 选项
    const char* opts[] = {"Resume", "Quit to Menu"};
    int start_y = py + 70;
    int opt_h = 42;
    int opt_w = 220;

    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))    sel = (sel - 1 + 2) % 2;
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))  sel = (sel + 1) % 2;

    for (int i = 0; i < 2; i++) {
        int ox = (WINDOW_WIDTH - opt_w) / 2;
        int oy = start_y + i * (opt_h + 6);
        bool s = (i == sel);

        Color bg = s ? Color{45, 50, 85, 255} : Color{30, 33, 58, 230};
        DrawRoundRect(ox, oy, opt_w, opt_h, 8, bg);

        Color tc = s ? TEXT_ACCENT : TEXT_DIM;
        int tw2 = MeasureText(opts[i], 18);
        DrawText(opts[i], ox + (opt_w - tw2) / 2, oy + opt_h / 2 - 9, 18, tc);

        if (s) {
            DrawRectangleLinesEx({(float)ox, (float)oy, (float)opt_w, (float)opt_h}, 2, TEXT_ACCENT);
        }
    }

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        int result = sel;
        sel = 0;
        return result;
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        sel = 0;
        return 0; // ESC = resume
    }
    return -1;
}

// ============================================================
// 游戏结束弹窗
// ============================================================
int Renderer_DrawGameOverPopup(GameState* state) {
    static int sel = 0;

    // 遮罩
    DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, {0, 0, 0, 200});

    int pw = 400, ph = 320;
    int px = (WINDOW_WIDTH - pw) / 2;
    int py = (WINDOW_HEIGHT - ph) / 2;

    DrawRoundRect(px, py, pw, ph, 14, {22, 25, 48, 250});
    DrawRectangleLinesEx({(float)px, (float)py, (float)pw, (float)ph}, 2, PANEL_BORDER);

    // 标题
    const char* title = "GAME OVER";
    int tw = MeasureText(title, 34);
    DrawText(title, (WINDOW_WIDTH - tw) / 2, py + 20, 34, TEXT_ACCENT);

    // 结果
    int winner = Game_GetWinner(state);
    const char* result;
    Color rc;
    if (!state->has_snake2) {
        result = "Good try!";
        rc = TEXT_PRIMARY;
    } else if (winner == 0) {
        result = "It's a Draw!";
        rc = TEXT_GOLD;
    } else if (winner == 1) {
        result = "Player 1 Wins!";
        rc = SNAKE1_HEAD;
    } else {
        result = state->snake2_is_ai ? "AI Wins!" : "Player 2 Wins!";
        rc = SNAKE2_HEAD;
    }
    tw = MeasureText(result, 22);
    DrawText(result, (WINDOW_WIDTH - tw) / 2, py + 58, 22, rc);

    // 分数
    char sbuf[64];
    snprintf(sbuf, sizeof(sbuf), "P1: %d pts  |  len: %d", state->snake1.score, state->snake1.length);
    tw = MeasureText(sbuf, 16);
    DrawText(sbuf, (WINDOW_WIDTH - tw) / 2, py + 95, 16, SNAKE1_BODY);

    if (state->has_snake2) {
        snprintf(sbuf, sizeof(sbuf), "%s: %d pts  |  len: %d",
                 state->snake2_is_ai ? "AI" : "P2",
                 state->snake2.score, state->snake2.length);
        tw = MeasureText(sbuf, 16);
        DrawText(sbuf, (WINDOW_WIDTH - tw) / 2, py + 118, 16, SNAKE2_BODY);
    }

    // 分隔线
    DrawLine(px + 40, py + 145, px + pw - 40, py + 145, {60, 65, 100, 200});

    // 选项
    const char* opts[] = {"Play Again", "Quit to Menu"};
    int start_y = py + 160;
    int opt_h = 42;
    int opt_w = 250;

    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))    sel = (sel - 1 + 2) % 2;
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))  sel = (sel + 1) % 2;

    for (int i = 0; i < 2; i++) {
        int ox = (WINDOW_WIDTH - opt_w) / 2;
        int oy = start_y + i * (opt_h + 6);
        bool s = (i == sel);

        Color bg = s ? Color{45, 50, 85, 255} : Color{30, 33, 58, 230};
        DrawRoundRect(ox, oy, opt_w, opt_h, 8, bg);

        Color tc = s ? TEXT_ACCENT : TEXT_DIM;
        int tw2 = MeasureText(opts[i], 18);
        DrawText(opts[i], ox + (opt_w - tw2) / 2, oy + opt_h / 2 - 9, 18, tc);

        if (s) {
            DrawRectangleLinesEx({(float)ox, (float)oy, (float)opt_w, (float)opt_h}, 2, TEXT_ACCENT);
        }
    }

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        int result = sel;
        sel = 0;
        return result;
    }
    return -1;
}
