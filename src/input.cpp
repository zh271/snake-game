#include "input.h"
#include "raylib.h"
#include "snake_music.h"
void Input_Init() {
    // raylib 输入无需特殊初始化
}

Direction Input_GetPlayer1Direction(bool versus_pvp) {
    if (IsKeyDown(KEY_W))    return DIR_UP;
    if (IsKeyDown(KEY_S))    return DIR_DOWN;
    if (IsKeyDown(KEY_A))    return DIR_LEFT;
    if (IsKeyDown(KEY_D))    return DIR_RIGHT;
    // 单人/人机模式下也支持方向键；双人 PvP 下 P1 只用 WASD
    if (!versus_pvp) {
        if (IsKeyDown(KEY_UP))    return DIR_UP;
        if (IsKeyDown(KEY_DOWN))  return DIR_DOWN;
        if (IsKeyDown(KEY_LEFT))  return DIR_LEFT;
        if (IsKeyDown(KEY_RIGHT)) return DIR_RIGHT;
    }
    return DIR_NONE;
}

Direction Input_GetPlayer2Direction() {
    // P2 只使用方向键
    if (IsKeyDown(KEY_UP))    return DIR_UP;
    if (IsKeyDown(KEY_DOWN))  return DIR_DOWN;
    if (IsKeyDown(KEY_LEFT))  return DIR_LEFT;
    if (IsKeyDown(KEY_RIGHT)) return DIR_RIGHT;
    return DIR_NONE;
}

bool Input_IsPausePressed() {
    return IsKeyPressed(KEY_P);
}

bool Input_IsQuitPressed() {
    return IsKeyPressed(KEY_Q);
}

bool Input_IsConfirmPressed() {
    return IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE);
}
