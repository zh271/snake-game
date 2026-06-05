#pragma once
#include "common.h"

// ============================================================
// 游戏核心逻辑 —— 负责蛇移动、碰撞、吃食物、死亡判定
// ============================================================

// 初始化整个游戏状态
void Game_Init(GameState* state, GameMode mode);

// 初始化一条蛇
void Snake_Init(Snake* snake, Point start_pos, Direction dir);

// 每帧更新（delta_ms: 距上一帧的毫秒数）
void Game_Update(GameState* state, int delta_ms);

// 蛇移动一格
void Snake_Move(GameState* state, Snake* snake);

// 设置蛇的下一步方向（禁止反向）
void Snake_SetDirection(Snake* snake, Direction dir);

// 判断两个方向是否相反
bool Direction_IsOpposite(Direction a, Direction b);

// 根据位置和方向计算下一格坐标
Point Position_Next(Point pos, Direction dir);

// 撞墙判定
bool Game_CheckWallCollision(Point pos);

// 撞障碍物判定（有钻头则不死）
bool Game_CheckObstacleCollision(GameState* state, Snake* snake, Point head_pos);

// 撞自己身体判定
bool Snake_CheckSelfCollision(Snake* snake, Point head_pos);

// 蛇头撞另一条蛇的身体
bool Snake_CheckOtherBodyCollision(Snake* attacker, Snake* target);

// 蛇头相撞
bool Snake_CheckHeadCollision(Snake* a, Snake* b);

// 吃普通食物
void Game_HandleEatFood(GameState* state, Snake* snake);

// 判定游戏结束
void Game_CheckGameOver(GameState* state);

// 获取胜利者: 0=平局/进行中, 1=蛇1胜, 2=蛇2胜
int Game_GetWinner(GameState* state);
