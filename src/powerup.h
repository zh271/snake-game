#pragma once
#include "common.h" // 确保 common.h 中去掉了 typedef，直接使用 struct GameState 等

class PowerUpManager {
private:
    // 内部辅助函数，不对外开放
    Position getRandomEmptyCell(GameState& state);
    PowerUpType getRandomType();
    bool isInMagnetRange(Position head, Position food);

public:
    PowerUpManager(); // 构造函数，用来初始化随机种子

    void init(GameState& state);
    void maintain(GameState& state);
    void spawn(GameState& state, int index);
    
    // 核心碰撞碰撞检测：判断蛇是否吃到道具
    bool checkEat(GameState& state, Snake& snake);
    
    // 应用道具效果
    void apply(GameState& state, Snake& snake, PowerUpType type);
    
    // 各个道具的具体实现
    void applyBigFood(GameState& state, Snake& snake);
    void applyMagnet(Snake& snake);
    void applyDrill(Snake& snake);
    void applyDigest(Snake& snake);
    void applySlow(Snake& snake);

    // 状态更新与持续效果处理
    void updateEffectAfterEating(Snake& snake);
    void handleMagnet(GameState& state, Snake& snake);
};
// ============================================================
// 全局自由函数接口（供 game_core 等模块调用）
// ============================================================
void PowerUp_Init(GameState* state);
void PowerUp_Maintain(GameState* state);
void PowerUp_Spawn(GameState* state, int index);
bool PowerUp_CheckEat(GameState* state, Snake* snake);
void PowerUp_Apply(GameState* state, Snake* snake, PowerUpType type);
void PowerUp_UpdateEffectAfterEating(Snake* snake);
void PowerUp_HandleMagnet(GameState* state, Snake* snake);
