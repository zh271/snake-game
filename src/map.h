#pragma once
#include "common.h"

// ============================================================
// 地图系统 —— 障碍物、食物生成、格子查询
// ============================================================

// 初始化地图（全部清为空地）
void Map_Init(Map* map);

// 随机生成 count 个障碍物
void Map_GenerateObstacles(GameState* state, int count);

// 在空格子生成普通食物
void Map_GenerateFood(GameState* state);

// 坐标是否在地图范围内
bool Map_IsInside(Point pos);

// 某个格子是否为"空"（考虑蛇身、道具、障碍物等）
bool Map_IsCellEmpty(GameState* state, Point pos);

// 随机获取一个空格子
Point Map_GetRandomEmptyCell(GameState* state);

// 设置/获取格子类型
void     Map_SetCell(Map* map, Point pos, CellType type);
CellType Map_GetCell(Map* map, Point pos);

// 某位置是否被任意蛇占据
bool Map_IsOccupiedBySnake(GameState* state, Point pos);

// 某位置是否被指定蛇占据
bool Snake_ContainsPosition(Snake* snake, Point pos);
