#include "common.h"
#include "game_core.h"
#include "renderer.h"
#include "input.h"
#include "ai.h"
#include "raylib.h"
#include <cstdlib>
#include <ctime>

int main() {
    srand((unsigned)time(nullptr));
    Renderer_Init();
    Input_Init();

    GameState game;
    GameMode mode = MODE_BASIC;
    bool in_menu = true;
    bool running = true;

    while (running && !WindowShouldClose()) {
        // === 菜单循环 ===
        while (in_menu && running && !WindowShouldClose()) {
            Renderer_Clear();
            int sel = Renderer_DrawMenu();
            Renderer_Present();
            if (sel == -1) { running = false; break; }
            if (sel >= 0) { mode = (GameMode)sel; in_menu = false; }
        }
        if (!running || WindowShouldClose()) break;

        // === 开始游戏 ===
        Game_Init(&game, mode);

        // === 游戏主循环 ===
        while (game.status != GAME_OVER && running && !WindowShouldClose()) {
            float dt = Renderer_GetFrameTime();
            int delta_ms = (int)(dt * 1000.0f);

            // P 键暂停（仅在运行中响应）
            if (game.status == GAME_RUNNING && Input_IsPausePressed()) {
                game.status = GAME_PAUSED;
            }

            // 读取输入（仅在运行中）
            if (game.status == GAME_RUNNING) {
                bool is_pvp = (game.mode == MODE_VERSUS_PVP);
                Direction p1 = Input_GetPlayer1Direction(is_pvp);
                if (p1 != DIR_NONE) Snake_SetDirection(&game.snake1, p1);

                if (game.mode == MODE_VERSUS_PVP) {
                    Direction p2 = Input_GetPlayer2Direction();
                    if (p2 != DIR_NONE) Snake_SetDirection(&game.snake2, p2);
                }
                if (game.mode == MODE_VERSUS_AI) {
                    Direction ai = AI_GetNextDirection(&game, &game.snake2, &game.snake1);
                    Snake_SetDirection(&game.snake2, ai);
                }
                Game_Update(&game, delta_ms);
            }

            // 渲染
            Renderer_Clear();
            Renderer_DrawGame(&game);

            // 暂停弹窗
            if (game.status == GAME_PAUSED) {
                int act = Renderer_DrawPausePopup();
                if (act == 0) {
                    game.status = GAME_RUNNING;  // Resume
                } else if (act == 1) {
                    game.status = GAME_OVER;      // Quit to menu
                }
            }

            Renderer_Present();
        }

        // === 游戏结束弹窗 ===
        while (game.status == GAME_OVER && running && !WindowShouldClose()) {
            Renderer_Clear();
            Renderer_DrawGame(&game);
            int act = Renderer_DrawGameOverPopup(&game);
            Renderer_Present();

            if (act == 0) {
                // Play Again — 重新开始同模式
                Game_Init(&game, mode);
                break;
            } else if (act == 1) {
                // Quit to Menu
                in_menu = true;
                break;
            }
        }
    }

    Renderer_Destroy();
    return 0;
}
