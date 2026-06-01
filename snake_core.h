#ifndef SNAKE_CORE_H
#define SNAKE_CORE_H

#define MAP_WIDTH 50
#define MAP_HEIGHT 50
#define MAX_CELLS (MAP_WIDTH * MAP_HEIGHT)
#define INIT_SNAKE_LENGTH 3

/* 坐标点结构体 */
typedef struct {
    int x;  /* 横坐标 */
    int y;  /* 纵坐标 */
} Point;

/* 地图中的实体类型 */
typedef enum {
    ENTITY_EMPTY = 0,      /* 空地 */
    ENTITY_SNAKE_HEAD,     /* 蛇头 */
    ENTITY_SNAKE_BODY,     /* 蛇身 */
    ENTITY_FOOD,           /* 普通食物 */
    ENTITY_OBSTACLE,       /* 障碍物 */
    ENTITY_BIG_FOOD,       /* 超大食物 */
    ENTITY_MAGNET,         /* 磁铁 */
    ENTITY_DRILL,          /* 破壁钻头 */
    ENTITY_DIGEST,         /* 消食片 */
    ENTITY_SLOW            /* 减速道具 */
} EntityType;

/* 蛇移动方向 */
typedef enum {
    DIR_UP = 0,     /* 向上 */
    DIR_DOWN = 1,   /* 向下 */
    DIR_LEFT = 2,   /* 向左 */
    DIR_RIGHT = 3   /* 向右 */
} Direction;

/* 游戏地图结构体 */
typedef struct {
    int width;                                      /* 地图宽度 */
    int height;                                     /* 地图高度 */
    EntityType cells[MAP_HEIGHT][MAP_WIDTH];       /* 地图格子数组 */
} GameMap;

/* 蛇结构体 */
typedef struct {
    Point body[MAX_CELLS];      /* 蛇身体坐标数组，body[0] 是蛇头 */
    int length;                 /* 蛇当前长度 */
    int direction;              /* 蛇当前移动方向 */
    int score;                  /* 蛇当前分数 */

    int hasMagnet;              /* 是否拥有磁铁效果 */
    int hasDrill;               /* 是否拥有钻头效果 */
    int foodEatenSinceBuff;     /* 获得 buff 后吃掉的食物数量 */
    int moveInterval;           /* 移动间隔，用于减速效果 */
} Snake;

/*
函数功能：
    初始化地图，将地图所有格子设置为空地。

参数：
    map：需要初始化的地图指针。

返回值：
    无。
*/
void GameMap_initMap(GameMap *map);

/*
函数功能：
    在地图指定坐标放置实体。

参数：
    map：需要操作的地图指针。
    p：实体放置坐标。
    type：实体类型。

返回值：
    无。
*/
void GameMap_placeEntity(GameMap *map, Point p, EntityType type);

/*
函数功能：
    获取地图指定坐标上的实体类型。

参数：
    map：需要查询的地图指针。
    p：需要查询的坐标。

返回值：
    返回该坐标上的实体类型。
    如果坐标越界，返回 ENTITY_EMPTY。
*/
EntityType GameMap_getEntityAt(GameMap *map, Point p);

/*
函数功能：
    清空地图指定坐标上的实体。

参数：
    map：需要操作的地图指针。
    p：需要清空的坐标。

返回值：
    无。
*/
void GameMap_clearEntity(GameMap *map, Point p);

/*
函数功能：
    初始化蛇，默认长度为 3，默认方向向右。

参数：
    snake：需要初始化的蛇指针。
    startPos：蛇头初始坐标。

返回值：
    无。
*/
void Snake_init(Snake *snake, Point startPos);

/*
函数功能：
    设置蛇的移动方向。

参数：
    snake：需要设置方向的蛇指针。
    dir：新的移动方向。

返回值：
    无。
*/
void Snake_setDirection(Snake *snake, int dir);

/*
函数功能：
    控制蛇按照当前方向移动一格。

参数：
    snake：需要移动的蛇指针。

返回值：
    无。
*/
void Snake_move(Snake *snake);

/*
函数功能：
    让蛇增长指定节数。

参数：
    snake：需要增长的蛇指针。
    count：增长节数。

返回值：
    无。
*/
void Snake_grow(Snake *snake, int count);

/*
函数功能：
    将蛇的长度缩短为当前长度的 2/3，最少保留 2 节。

参数：
    snake：需要缩短的蛇指针。

返回值：
    无。
*/
void Snake_shrink(Snake *snake);

#endif
