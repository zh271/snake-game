#pragma once
#include "common.h"

// ============================================================
// 图形渲染系统 —— raylib 精美渲染
// ============================================================

// 窗口尺寸常量
constexpr int WINDOW_WIDTH  = 1060;
constexpr int WINDOW_HEIGHT = 820;
constexpr int CELL_SIZE     = 15;
constexpr int GRID_OFFSET_X = 30;
constexpr int GRID_OFFSET_Y = 30;
constexpr int PANEL_X       = 820;
constexpr int PANEL_WIDTH   = 220;

// 初始化图形窗口
void Renderer_Init();

// 清屏
void Renderer_Clear();

// 绘制整个游戏画面
void Renderer_DrawGame(GameState* state);

// 绘制地图网格
void Renderer_DrawGrid();

// 绘制障碍物
void Renderer_DrawObstacles(Map* map);

// 绘制食物（带动画）
void Renderer_DrawFood(Map* map, float anim_t);

// 绘制一条蛇
void Renderer_DrawSnake(Snake* snake, int player_id, float anim_t);

// 绘制道具
void Renderer_DrawPowerUps(GameState* state, float anim_t);

// 绘制侧边栏 UI
void Renderer_DrawUI(GameState* state);

// 绘制游戏结束画面
void Renderer_DrawGameOver(GameState* state);

// 绘制主菜单
int Renderer_DrawMenu();  // 返回选中的模式

// 刷新画面（双缓冲提交）
void Renderer_Present();

// 关闭图形
void Renderer_Destroy();

// 获取帧时间
float Renderer_GetFrameTime();

// 绘制暂停弹窗（返回：0=继续，1=退出主菜单，-1=选择中）
int Renderer_DrawPausePopup();

// 绘制游戏结束弹窗（返回：0=再来一局，1=退出主菜单，-1=选择中）
int Renderer_DrawGameOverPopup(GameState* state);
