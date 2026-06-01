#include "snake_core.h"

/*
函数名称：isReverseDirection

函数功能：
    判断两个方向是否互为相反方向。
    在贪吃蛇游戏中，蛇不能直接反向移动。
    例如当前向右移动时，不能直接改成向左移动。

传入参数：
    oldDir：
        当前蛇的移动方向。
        可取值：
            DIR_UP    表示向上
            DIR_DOWN  表示向下
            DIR_LEFT  表示向左
            DIR_RIGHT 表示向右

    newDir：
        想要设置的新移动方向。
        可取值同 oldDir。

返回值：
    返回 1：
        表示 oldDir 和 newDir 是相反方向。

    返回 0：
        表示 oldDir 和 newDir 不是相反方向。
*/
static int isReverseDirection(int oldDir, int newDir) {
    if (oldDir == DIR_UP && newDir == DIR_DOWN) return 1;
    if (oldDir == DIR_DOWN && newDir == DIR_UP) return 1;
    if (oldDir == DIR_LEFT && newDir == DIR_RIGHT) return 1;
    if (oldDir == DIR_RIGHT && newDir == DIR_LEFT) return 1;
    return 0;
}

/*
函数名称：wrapPoint

函数功能：
    对坐标进行“穿墙”处理。
    当蛇头移动到地图边界外时，让它从地图另一侧出现。

    例如：
        x < 0 时，表示从左边界出去，此时让 x 变成 MAP_WIDTH - 1。
        x >= MAP_WIDTH 时，表示从右边界出去，此时让 x 变成 0。
        y < 0 时，表示从上边界出去，此时让 y 变成 MAP_HEIGHT - 1。
        y >= MAP_HEIGHT 时，表示从下边界出去，此时让 y 变成 0。

传入参数：
    p：
        需要处理的坐标点。
        p.x 表示横坐标。
        p.y 表示纵坐标。

返回值：
    返回处理后的 Point 坐标。
    如果原坐标没有越界，则返回原坐标。
    如果原坐标越界，则返回穿墙后的坐标。
*/
static Point wrapPoint(Point p) {
    if (p.x < 0) p.x = MAP_WIDTH - 1;
    if (p.x >= MAP_WIDTH) p.x = 0;
    if (p.y < 0) p.y = MAP_HEIGHT - 1;
    if (p.y >= MAP_HEIGHT) p.y = 0;
    return p;
}

/*
函数名称：getNextPoint

函数功能：
    根据当前蛇头坐标和移动方向，计算蛇头下一步应该到达的位置。
    该函数内部会调用 wrapPoint，因此包含穿墙逻辑。

传入参数：
    head：
        当前蛇头坐标。
        head.x 表示蛇头当前横坐标。
        head.y 表示蛇头当前纵坐标。

    direction：
        当前移动方向。
        可取值：
            DIR_UP    表示向上移动，y 减 1
            DIR_DOWN  表示向下移动，y 加 1
            DIR_LEFT  表示向左移动，x 减 1
            DIR_RIGHT 表示向右移动，x 加 1

返回值：
    返回蛇头下一步的位置坐标。
    如果下一步越过地图边界，则返回穿墙后的坐标。
*/
static Point getNextPoint(Point head, int direction) {
    Point next = head;

    if (direction == DIR_UP) {
        next.y--;
    } else if (direction == DIR_DOWN) {
        next.y++;
    } else if (direction == DIR_LEFT) {
        next.x--;
    } else if (direction == DIR_RIGHT) {
        next.x++;
    }

    return wrapPoint(next);
}

/* =========================
   GameMap 相关函数实现
   ========================= */

/*
函数名称：GameMap_initMap

函数功能：
    初始化游戏地图。
    设置地图宽度和高度，并将地图中所有格子初始化为空地 ENTITY_EMPTY。

传入参数：
    map：
        GameMap 类型的指针，指向需要初始化的地图变量。
        通过指针传入，可以直接修改原地图的数据。

返回值：
    无返回值。
    函数执行后，map 指向的地图会被初始化为 50 x 50 的空地图。
*/
void GameMap_initMap(GameMap *map) {
    int i, j;

    map->width = MAP_WIDTH;
    map->height = MAP_HEIGHT;

    for (i = 0; i < MAP_HEIGHT; i++) {
        for (j = 0; j < MAP_WIDTH; j++) {
            map->cells[i][j] = ENTITY_EMPTY;
        }
    }
}

/*
函数名称：GameMap_placeEntity

函数功能：
    在地图指定坐标位置放置一个实体。
    实体可以是蛇头、蛇身、食物、障碍物或道具等。

传入参数：
    map：
        GameMap 类型的指针，表示要操作的地图。

    p：
        Point 类型坐标，表示实体要放置的位置。
        p.x 表示横坐标，对应地图列。
        p.y 表示纵坐标，对应地图行。

    type：
        EntityType 类型，表示要放置的实体类型。
        例如：
            ENTITY_EMPTY      空地
            ENTITY_SNAKE_HEAD 蛇头
            ENTITY_SNAKE_BODY 蛇身
            ENTITY_FOOD       普通食物
            ENTITY_OBSTACLE   障碍物
            ENTITY_BIG_FOOD   超大食物
            ENTITY_MAGNET     磁铁
            ENTITY_DRILL      破壁钻头
            ENTITY_DIGEST     消食片
            ENTITY_SLOW       减速道具

返回值：
    无返回值。
    如果坐标合法，则修改地图对应位置的实体类型。
    如果坐标越界，则不进行任何操作。
*/
void GameMap_placeEntity(GameMap *map, Point p, EntityType type) {
    /* 坐标越界则不处理，防止访问数组越界 */
    if (p.x < 0 || p.x >= MAP_WIDTH || p.y < 0 || p.y >= MAP_HEIGHT) {
        return;
    }

    map->cells[p.y][p.x] = type;
}

/*
函数名称：GameMap_getEntityAt

函数功能：
    获取地图指定坐标位置上的实体类型。

传入参数：
    map：
        GameMap 类型的指针，表示要查询的地图。

    p：
        Point 类型坐标，表示要查询的位置。
        p.x 表示横坐标，对应地图列。
        p.y 表示纵坐标，对应地图行。

返回值：
    返回 EntityType 类型的值，表示该坐标上的实体类型。

    如果坐标合法：
        返回 map->cells[p.y][p.x] 中保存的实体类型。

    如果坐标越界：
        返回 ENTITY_EMPTY，表示默认当作空地处理。
*/
EntityType GameMap_getEntityAt(GameMap *map, Point p) {
    /* 如果坐标越界，默认返回空地 */
    if (p.x < 0 || p.x >= MAP_WIDTH || p.y < 0 || p.y >= MAP_HEIGHT) {
        return ENTITY_EMPTY;
    }

    return map->cells[p.y][p.x];
}

/*
函数名称：GameMap_clearEntity

函数功能：
    清空地图指定坐标位置上的实体。
    实际上就是把该位置设置为 ENTITY_EMPTY。

传入参数：
    map：
        GameMap 类型的指针，表示要操作的地图。

    p：
        Point 类型坐标，表示需要清空的位置。
        p.x 表示横坐标，对应地图列。
        p.y 表示纵坐标，对应地图行。

返回值：
    无返回值。
    如果坐标合法，则将该位置设置为空地。
    如果坐标越界，则不进行任何操作。
*/
void GameMap_clearEntity(GameMap *map, Point p) {
    /* 坐标越界则不处理，防止访问数组越界 */
    if (p.x < 0 || p.x >= MAP_WIDTH || p.y < 0 || p.y >= MAP_HEIGHT) {
        return;
    }

    map->cells[p.y][p.x] = ENTITY_EMPTY;
}

/* =========================
   Snake 相关函数实现
   ========================= */

/*
函数名称：Snake_init

函数功能：
    初始化一条蛇。
    初始化内容包括：
        1. 设置蛇的初始长度为 INIT_SNAKE_LENGTH，即 3。
        2. 设置默认移动方向为向右。
        3. 设置初始分数为 0。
        4. 清空道具状态。
        5. 设置移动间隔为 1。
        6. 设置蛇的身体坐标。
           蛇头放在 startPos，身体向左排列。

传入参数：
    snake：
        Snake 类型的指针，指向需要初始化的蛇变量。
        通过指针传入，可以直接修改原蛇的数据。

    startPos：
        Point 类型坐标，表示蛇头的初始位置。
        startPos.x 表示蛇头初始横坐标。
        startPos.y 表示蛇头初始纵坐标。

返回值：
    无返回值。
    函数执行后，snake 指向的蛇会被初始化。
*/
void Snake_init(Snake *snake, Point startPos) {
    int i;

    snake->length = INIT_SNAKE_LENGTH;
    snake->direction = DIR_RIGHT;
    snake->score = 0;

    snake->hasMagnet = 0;
    snake->hasDrill = 0;
    snake->foodEatenSinceBuff = 0;
    snake->moveInterval = 1;

    /*
        初始化蛇身坐标。
        body[0] 是蛇头，放在 startPos。
        body[1] 在蛇头左边一格。
        body[2] 在蛇头左边两格。
    */
    for (i = 0; i < INIT_SNAKE_LENGTH; i++) {
        snake->body[i].x = startPos.x - i;
        snake->body[i].y = startPos.y;
    }
}

/*
函数名称：Snake_setDirection

函数功能：
    设置蛇的移动方向。
    该函数会检查新方向是否合法：
        1. 方向必须是 DIR_UP、DIR_DOWN、DIR_LEFT、DIR_RIGHT 之一。
        2. 新方向不能和当前方向相反，防止蛇直接掉头撞到自己。

传入参数：
    snake：
        Snake 类型的指针，表示要修改方向的蛇。

    dir：
        要设置的新方向。
        可取值：
            DIR_UP    表示向上
            DIR_DOWN  表示向下
            DIR_LEFT  表示向左
            DIR_RIGHT 表示向右

返回值：
    无返回值。
    如果方向合法，则修改 snake->direction。
    如果方向非法或为相反方向，则不做修改。
*/
void Snake_setDirection(Snake *snake, int dir) {
    if (dir < DIR_UP || dir > DIR_RIGHT) {
        return;
    }

    if (isReverseDirection(snake->direction, dir)) {
        return;
    }

    snake->direction = dir;
}

/*
函数名称：Snake_move

函数功能：
    控制蛇按照当前方向移动一次。

    移动规则：
        1. 根据蛇头当前位置和当前方向计算新的蛇头位置。
        2. 蛇身从尾部开始，依次移动到前一节的位置。
        3. 最后把新的蛇头位置赋给 body[0]。
        4. 如果蛇头越过地图边界，会通过穿墙逻辑从另一侧出现。

传入参数：
    snake：
        Snake 类型的指针，表示需要移动的蛇。
        函数会直接修改 snake->body 中的坐标。

返回值：
    无返回值。
    函数执行后，蛇整体向当前方向移动一格。
*/
void Snake_move(Snake *snake) {
    int i;
    Point nextHead = getNextPoint(snake->body[0], snake->direction);

    /*
        身体从后往前移动。
        例如：
            body[2] 移动到 body[1] 原来的位置
            body[1] 移动到 body[0] 原来的位置
    */
    for (i = snake->length - 1; i > 0; i--) {
        snake->body[i] = snake->body[i - 1];
    }

    /* 更新蛇头为新坐标 */
    snake->body[0] = nextHead;
}

/*
函数名称：Snake_grow

函数功能：
    让蛇增长指定节数。
    本函数采用简单增长方式：
        每增长一节，就在蛇尾后面复制一节。
        新增的身体坐标暂时和原蛇尾坐标相同。
        下一次移动后，蛇身会自然拉开。

传入参数：
    snake：
        Snake 类型的指针，表示需要增长的蛇。

    count：
        要增长的节数。
        例如：
            count = 1 表示增长 1 节。
            count = 2 表示增长 2 节。
        如果 count <= 0，则不增长。

返回值：
    无返回值。
    函数执行后，snake->length 会增加。
    如果蛇长度已经达到 MAX_CELLS，则不会继续增长。
*/
void Snake_grow(Snake *snake, int count) {
    int i;

    if (count <= 0) {
        return;
    }

    for (i = 0; i < count; i++) {
        if (snake->length >= MAX_CELLS) {
            return;
        }

        /*
            在蛇尾增加一节。
            新增部分先复制当前蛇尾的位置。
        */
        snake->body[snake->length] = snake->body[snake->length - 1];
        snake->length++;
    }
}

/*
函数名称：Snake_shrink

函数功能：
    将蛇的长度缩短为当前长度的 2/3。
    例如：
        当前长度为 9，缩短后为 6。
        当前长度为 6，缩短后为 4。

    为了避免蛇太短，最少保留 2 节。

传入参数：
    snake：
        Snake 类型的指针，表示需要缩短的蛇。

返回值：
    无返回值。
    函数执行后，snake->length 会变为原来的 2/3。
    如果计算后小于 2，则设置为 2。
*/
void Snake_shrink(Snake *snake) {
    int newLength = snake->length * 2 / 3;

    if (newLength < 2) {
        newLength = 2;
    }

    snake->length = newLength;
}
