#pragma once
#include "common.h"

// ============================================================
// AI 系统 —— 为 AI 蛇选择移动方向
// ============================================================

// 主决策：根据当前状态返回 AI 下一步方向
Direction AI_GetNextDirection(GameState* state, Snake* ai_snake, Snake* enemy_snake);

// 判断某方向对 AI 蛇是否安全
bool AI_IsDirectionSafe(GameState* state, Snake* ai_snake, Snake* enemy_snake, Direction dir);

// 曼哈顿距离
int AI_ManhattanDistance(Point a, Point b);

// 获取所有安全方向，返回数量
int AI_GetSafeDirections(GameState* state, Snake* ai_snake, Snake* enemy_snake, Direction safe_dirs[4]);

// 从安全方向中选择最接近食物的
Direction AI_ChooseDirectionToFood(GameState* state, Snake* ai_snake, Direction safe_dirs[4], int safe_count);

// 随机选一个安全方向
Direction AI_ChooseRandomSafeDirection(Direction safe_dirs[4], int safe_count);
