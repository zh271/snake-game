#pragma once
#include "common.h"

// ============================================================
// 输入系统 —— 键盘 → 方向
// ============================================================

// 初始化输入（raylib 窗口创建后调用）
void Input_Init();

// 读取玩家1方向
// versus_pvp=true: 仅WASD（双人对战）; false: WASD+方向键均可
Direction Input_GetPlayer1Direction(bool versus_pvp = false);

// 读取玩家2方向（仅方向键，双人对战时与P1区分）
Direction Input_GetPlayer2Direction();

// 是否按下暂停
bool Input_IsPausePressed();

// 是否按下退出
bool Input_IsQuitPressed();

// 是否按下确认/回车
bool Input_IsConfirmPressed();
