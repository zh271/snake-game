#pragma once

// ---------------------------------------------------------
// 1. 全局常量定义
// ---------------------------------------------------------
constexpr int MAP_WIDTH = 50;
constexpr int MAP_HEIGHT = 50;
constexpr int MAX_SNAKE_LENGTH = MAP_WIDTH * MAP_HEIGHT;
constexpr int OBSTACLE_COUNT = 20;
constexpr int POWERUP_ON_MAP = 2;
constexpr int INIT_SNAKE_LENGTH = 3;
constexpr int NORMAL_MOVE_INTERVAL_MS = 1000; // 1秒移动一格
constexpr int SLOW_MOVE_INTERVAL_MS = 2000;   // 减速：2秒移动一格

// ---------------------------------------------------------
// 2. 基础数据类型
// ---------------------------------------------------------

// 坐标点结构体 (统一命名为 Point)
struct Point {
    int x;
    int y;

    // 默认构造函数：初始化为 (-1, -1) 代表无效坐标
    Point(int _x = -1, int _y = -1) : x(_x), y(_y) {}

    //重载等于号，方便直接对比两个坐标是否相同
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
    bool operator!=(const Point& other) const {
        return !(*this == other);
    }
};

using Position = Point;

// 方向枚举
enum Direction {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT,
    DIR_NONE
};

// 游戏模式枚举
enum GameMode {
    MODE_BASIC,        // 基础版
    MODE_POWERUP,      // 道具版
    MODE_VERSUS_PVP,   // 双人对战
    MODE_VERSUS_AI     // 人机对战
};

// 地图格子类型枚举
enum CellType {
    CELL_EMPTY,       // 空地
    CELL_OBSTACLE,    // 障碍物
    CELL_FOOD,        // 普通食物
    CELL_POWERUP      // 道具
};

// 道具类型枚举
enum PowerUpType {
    POWER_NONE,
    POWER_BIG_FOOD,   // 超大食物
    POWER_MAGNET,     // 磁铁
    POWER_DRILL,      // 破壁钻头
    POWER_DIGEST,     // 消食片
    POWER_SLOW        // 减速
};

// 游戏状态枚举
enum GameStatus {
    GAME_RUNNING,      
    GAME_OVER,         
    GAME_PAUSED        
};

// ---------------------------------------------------------
// 3. 核心游戏对象
// ---------------------------------------------------------

// 蛇结构体
struct Snake {
    Point body[MAX_SNAKE_LENGTH]; // body[0] 是蛇头
    int length;                   // 当前长度
    Direction dir;                // 当前移动方向
    Direction next_dir;           // 玩家下一步输入方向
    int score;                    // 当前分数
    bool alive;                   // 是否存活

    // 道具状态 (剩余需要吃几个食物)
    int magnet_food_left;
    int drill_food_left;
    int slow_food_left;
    int move_interval_ms;

    // 默认构造函数：每次创建 Snake，自动完成标准化初始化
    Snake() : 
        length(INIT_SNAKE_LENGTH), 
        dir(DIR_NONE), 
        next_dir(DIR_NONE), 
        score(0), 
        alive(true),
        magnet_food_left(0), 
        drill_food_left(0), 
        slow_food_left(0), 
        move_interval_ms(NORMAL_MOVE_INTERVAL_MS) {}
};

// 地图结构体
struct Map {
    CellType cells[MAP_HEIGHT][MAP_WIDTH];
    Point food_pos;

    // 自动将 50x50 地图全部刷为空地
    Map() {
        for (int i = 0; i < MAP_HEIGHT; i++) {
            for (int j = 0; j < MAP_WIDTH; j++) {
                cells[i][j] = CELL_EMPTY;
            }
        }
    }
};

// 道具实体结构体
struct PowerUp {
    PowerUpType type;
    Point pos;
    bool active;

    // 默认构造
    PowerUp() : type(POWER_NONE), active(false) {}
};

// 全局游戏状态结构体
struct GameState {
    GameMode mode;             
    GameStatus status;         

    Map map;                   
    Snake snake1;              
    Snake snake2;              

    PowerUp powerups[POWERUP_ON_MAP];  

    int step_count;            
    int random_event_step;     

    bool has_snake2;           
    bool snake2_is_ai;         

    // 默认构造函数：初始化状态机，确保游戏不会一启动就崩溃
    GameState() : 
        mode(MODE_BASIC), 
        status(GAME_RUNNING), 
        step_count(0), 
        random_event_step(0), 
        has_snake2(false), 
        snake2_is_ai(false) {}
};