#include "common.h"

#include <wx/wx.h>
#include <wx/dcbuffer.h>
#include <algorithm>

// =========================================================
// 1. wxWidgets 应用对象
// =========================================================
//
// 作用：
// - wxWidgets 程序必须存在 wxApp 对象。
// - 这里使用 wxIMPLEMENT_APP_NO_MAIN，使 Renderer_Init()
//   可以在普通 main 函数中手动初始化 wxWidgets。
//
// 如果你的工程已经有自己的 wxApp，请删除本段。
// =========================================================

class RendererWxApp : public wxApp {
public:
    bool OnInit() override {
        return true;
    }
};

wxIMPLEMENT_APP_NO_MAIN(RendererWxApp);


// =========================================================
// 2. 渲染器内部全局变量
// =========================================================
//
// 说明：
// - 这些变量只在 renderer.cpp 内部使用。
// - 外部游戏逻辑只需要调用 renderer.h 中声明的函数。
// =========================================================

// 窗口和画布
static wxFrame* g_frame = nullptr;
static wxPanel* g_canvas = nullptr;

// 双缓冲画布
static wxBitmap g_backBuffer;
static wxMemoryDC g_memDC;

// 是否由 Renderer_Init 初始化了 wxWidgets
static bool g_wxStartedByRenderer = false;

// 窗口尺寸
static int g_windowWidth = 800;
static int g_windowHeight = 700;

// UI 区域高度
static constexpr int UI_HEIGHT = 90;

// 地图边距
static constexpr int BOARD_MARGIN = 12;

// 单个地图格子的像素大小
static int g_cellSize = 10;

// 地图绘制区域左上角
static int g_boardLeft = 0;
static int g_boardTop = 0;

// 地图绘制区域像素尺寸
static int g_boardPixelWidth = 0;
static int g_boardPixelHeight = 0;

// UI 区域顶部位置
static int g_uiTop = 0;


// =========================================================
// 3. 颜色定义
// =========================================================

static const wxColour COLOR_BACKGROUND(245, 247, 250);
static const wxColour COLOR_BOARD(255, 255, 255);
static const wxColour COLOR_GRID(225, 230, 235);
static const wxColour COLOR_OBSTACLE(70, 70, 70);
static const wxColour COLOR_FOOD(220, 70, 70);

static const wxColour COLOR_PLAYER1_BODY(70, 180, 90);
static const wxColour COLOR_PLAYER1_HEAD(30, 130, 60);

static const wxColour COLOR_PLAYER2_BODY(80, 130, 220);
static const wxColour COLOR_PLAYER2_HEAD(40, 80, 170);

static const wxColour COLOR_TEXT(40, 45, 55);


// =========================================================
// 4. 自定义画布类
// =========================================================
//
// 作用：
// - 接收 wxWidgets 的绘图事件。
// - 将 g_backBuffer 中已经绘制好的图像复制到窗口。
// =========================================================

class RendererCanvas : public wxPanel {
public:
    explicit RendererCanvas(wxWindow* parent)
        : wxPanel(parent, wxID_ANY) {

        // 使用自绘背景，减少闪烁
        SetBackgroundStyle(wxBG_STYLE_PAINT);
    }

private:
    void OnPaint(wxPaintEvent&) {
        wxAutoBufferedPaintDC dc(this);
        dc.SetBackground(wxBrush(COLOR_BACKGROUND));
        dc.Clear();

        if (g_backBuffer.IsOk()) {
            dc.DrawBitmap(g_backBuffer, 0, 0, false);
        }
    }

    wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(RendererCanvas, wxPanel)
    EVT_PAINT(RendererCanvas::OnPaint)
wxEND_EVENT_TABLE()


// =========================================================
// 5. 内部辅助函数
// =========================================================

/*
 * 判断地图坐标是否合法。
 *
 * 参数：
 * - p：地图坐标
 *
 * 返回：
 * - true：坐标在地图范围内
 * - false：坐标越界
 */
static bool Renderer_IsValidPoint(const Point& p) {
    return p.x >= 0 && p.x < MAP_WIDTH &&
           p.y >= 0 && p.y < MAP_HEIGHT;
}


/*
 * 将地图坐标转换为屏幕像素矩形。
 *
 * 参数：
 * - p：地图坐标，例如 (10, 20)
 *
 * 返回：
 * - wxRect：对应屏幕上的矩形区域
 *
 * 算法：
 * - 屏幕 x = 地图左边距 + 地图 x * 单元格大小
 * - 屏幕 y = 地图上边距 + 地图 y * 单元格大小
 */
static wxRect Renderer_CellRect(const Point& p) {
    return wxRect(
        g_boardLeft + p.x * g_cellSize,
        g_boardTop + p.y * g_cellSize,
        g_cellSize,
        g_cellSize
    );
}


/*
 * 绘制一个地图格子。
 *
 * 参数：
 * - p：地图坐标
 * - fill：填充颜色
 * - border：边框颜色
 *
 * 功能：
 * - 用统一方式绘制障碍物、蛇身体等方块类元素。
 */
static void Renderer_DrawCell(
    const Point& p,
    const wxColour& fill,
    const wxColour& border
) {
    if (!Renderer_IsValidPoint(p)) {
        return;
    }

    wxRect rect = Renderer_CellRect(p);

    int padding = std::max(1, g_cellSize / 10);
    rect.Deflate(padding, padding);

    g_memDC.SetBrush(wxBrush(fill));
    g_memDC.SetPen(wxPen(border));
    g_memDC.DrawRoundedRectangle(rect, std::max(2, g_cellSize / 5));
}


/*
 * 在指定矩形中居中绘制文字。
 *
 * 参数：
 * - text：文字内容
 * - rect：目标矩形
 * - color：文字颜色
 */
static void Renderer_DrawCenteredText(
    const wxString& text,
    const wxRect& rect,
    const wxColour& color
) {
    g_memDC.SetTextForeground(color);

    wxSize size = g_memDC.GetTextExtent(text);

    int x = rect.x + (rect.width - size.GetWidth()) / 2;
    int y = rect.y + (rect.height - size.GetHeight()) / 2;

    g_memDC.DrawText(text, x, y);
}


/*
 * 根据游戏模式返回显示文本。
 */
static wxString Renderer_GameModeName(GameMode mode) {
    switch (mode) {
    case MODE_BASIC:
        return wxString::FromUTF8("基础版");
    case MODE_POWERUP:
        return wxString::FromUTF8("道具版");
    case MODE_VERSUS_PVP:
        return wxString::FromUTF8("双人对战");
    case MODE_VERSUS_AI:
        return wxString::FromUTF8("人机对战");
    default:
        return wxString::FromUTF8("未知模式");
    }
}


/*
 * 根据道具类型返回显示文字。
 */
static wxString Renderer_PowerUpLabel(PowerUpType type) {
    switch (type) {
    case POWER_BIG_FOOD:
        return wxString::FromUTF8("大");
    case POWER_MAGNET:
        return wxString::FromUTF8("磁");
    case POWER_DRILL:
        return wxString::FromUTF8("钻");
    case POWER_DIGEST:
        return wxString::FromUTF8("消");
    case POWER_SLOW:
        return wxString::FromUTF8("慢");
    default:
        return wxString::FromUTF8("?");
    }
}


/*
 * 根据道具类型返回颜色。
 */
static wxColour Renderer_PowerUpColor(PowerUpType type) {
    switch (type) {
    case POWER_BIG_FOOD:
        return wxColour(240, 150, 40);
    case POWER_MAGNET:
        return wxColour(180, 80, 220);
    case POWER_DRILL:
        return wxColour(90, 90, 90);
    case POWER_DIGEST:
        return wxColour(70, 180, 180);
    case POWER_SLOW:
        return wxColour(80, 160, 240);
    default:
        return wxColour(160, 160, 160);
    }
}


/*
 * 计算地图和 UI 区域布局。
 *
 * 参数：
 * - width：窗口宽度
 * - height：窗口高度
 *
 * 算法：
 * - 根据窗口大小和地图尺寸，计算每个格子的最大可用像素大小。
 * - 保证 50x50 地图完整显示。
 * - 下方预留 UI 区域。
 */
static void Renderer_UpdateLayout(int width, int height) {
    g_windowWidth = width;
    g_windowHeight = height;

    int boardAreaWidth = width - BOARD_MARGIN * 2;
    int boardAreaHeight = height - UI_HEIGHT - BOARD_MARGIN * 2;

    boardAreaWidth = std::max(boardAreaWidth, MAP_WIDTH);
    boardAreaHeight = std::max(boardAreaHeight, MAP_HEIGHT);

    g_cellSize = std::min(
        boardAreaWidth / MAP_WIDTH,
        boardAreaHeight / MAP_HEIGHT
    );

    g_cellSize = std::max(g_cellSize, 1);

    g_boardPixelWidth = MAP_WIDTH * g_cellSize;
    g_boardPixelHeight = MAP_HEIGHT * g_cellSize;

    g_boardLeft = (width - g_boardPixelWidth) / 2;
    g_boardTop = BOARD_MARGIN;

    g_uiTop = g_boardTop + g_boardPixelHeight + 8;
}


/*
 * 检查渲染器是否已经成功初始化。
 */
static bool Renderer_IsReady() {
    return g_frame != nullptr &&
           g_canvas != nullptr &&
           g_backBuffer.IsOk() &&
           g_memDC.IsOk();
}


// =========================================================
// 6. 对外接口实现
// =========================================================

/*
 * 初始化图形窗口。
 *
 * 接口定义：
 * void Renderer_Init(int width, int height);
 *
 * 参数：
 * - width：窗口宽度
 * - height：窗口高度
 *
 * 变量：
 * - g_frame：主窗口
 * - g_canvas：绘图面板
 * - g_backBuffer：双缓冲位图
 * - g_memDC：内存绘图 DC
 *
 * 算法实现：
 * 1. 初始化 wxWidgets。
 * 2. 创建窗口和绘图面板。
 * 3. 根据窗口大小计算地图布局。
 * 4. 创建双缓冲位图，并绑定到 wxMemoryDC。
 * 5. 清空画面并显示窗口。
 */
void Renderer_Init(int width, int height) {
    if (g_frame != nullptr) {
        return;
    }

    // 初始化 wxWidgets
    if (!wxTheApp) {
        wxApp::SetInstance(new RendererWxApp());

        int argc = 0;
        char** argv = nullptr;

        if (!wxEntryStart(argc, argv)) {
            return;
        }

        wxTheApp->CallOnInit();
        g_wxStartedByRenderer = true;
    }

    Renderer_UpdateLayout(width, height);

    g_frame = new wxFrame(
        nullptr,
        wxID_ANY,
        wxString::FromUTF8("Snake Game - wxWidgets"),
        wxDefaultPosition,
        wxDefaultSize,
        wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX)
    );

    g_frame->SetClientSize(width, height);

    g_canvas = new RendererCanvas(g_frame);
    g_canvas->SetMinSize(wxSize(width, height));

    // 创建双缓冲位图
    g_backBuffer = wxBitmap(width, height);

    // 将内存 DC 绑定到位图
    g_memDC.SelectObject(g_backBuffer);

    // 初始化字体
    g_memDC.SetFont(wxFont(
        11,
        wxFONTFAMILY_DEFAULT,
        wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_NORMAL
    ));

    g_frame->Show(true);

    Renderer_Clear();
    Renderer_Present();
}


/*
 * 清空屏幕。
 *
 * 接口定义：
 * void Renderer_Clear(void);
 *
 * 功能：
 * - 清空双缓冲画布。
 * - 设置整体背景色。
 *
 * 算法实现：
 * - 使用 wxMemoryDC 的 Clear() 清空 g_backBuffer。
 */
void Renderer_Clear(void) {
    if (!Renderer_IsReady()) {
        return;
    }

    g_memDC.SetBackground(wxBrush(COLOR_BACKGROUND));
    g_memDC.Clear();
}


/*
 * 绘制整个游戏画面。
 *
 * 接口定义：
 * void Renderer_DrawGame(GameState *state);
 *
 * 参数：
 * - state：当前游戏状态
 *
 * 功能：
 * - 绘制地图
 * - 绘制障碍物
 * - 绘制食物
 * - 绘制道具
 * - 绘制蛇
 * - 绘制 UI
 * - 游戏结束时绘制结束界面
 *
 * 算法实现：
 * 1. 清空画面。
 * 2. 绘制地图网格。
 * 3. 绘制地图元素。
 * 4. 绘制玩家蛇。
 * 5. 绘制 UI。
 * 6. 根据状态绘制暂停或结束覆盖层。
 */
void Renderer_DrawGame(GameState *state) {
    if (!Renderer_IsReady() || state == nullptr) {
        return;
    }

    Renderer_Clear();

    Renderer_DrawGrid();
    Renderer_DrawObstacles(&state->map);
    Renderer_DrawFood(&state->map);
    Renderer_DrawPowerUps(state);

    Renderer_DrawSnake(&state->snake1, 1);

    if (state->has_snake2) {
        Renderer_DrawSnake(&state->snake2, 2);
    }

    Renderer_DrawUI(state);

    if (state->status == GAME_OVER) {
        Renderer_DrawGameOver(state);
    } else if (state->status == GAME_PAUSED) {
        wxRect box(
            g_boardLeft + g_boardPixelWidth / 2 - 100,
            g_boardTop + g_boardPixelHeight / 2 - 35,
            200,
            70
        );

        g_memDC.SetBrush(wxBrush(wxColour(255, 255, 255)));
        g_memDC.SetPen(wxPen(wxColour(80, 80, 80), 2));
        g_memDC.DrawRoundedRectangle(box, 10);

        g_memDC.SetFont(wxFont(
            18,
            wxFONTFAMILY_DEFAULT,
            wxFONTSTYLE_NORMAL,
            wxFONTWEIGHT_BOLD
        ));

        Renderer_DrawCenteredText(
            wxString::FromUTF8("游戏暂停"),
            box,
            COLOR_TEXT
        );

        g_memDC.SetFont(wxFont(
            11,
            wxFONTFAMILY_DEFAULT,
            wxFONTSTYLE_NORMAL,
            wxFONTWEIGHT_NORMAL
        ));
    }
}


/*
 * 绘制地图网格。
 *
 * 接口定义：
 * void Renderer_DrawGrid(void);
 *
 * 功能：
 * - 绘制地图背景。
 * - 绘制 50x50 网格线。
 *
 * 变量：
 * - g_boardLeft / g_boardTop：地图左上角
 * - g_cellSize：单元格大小
 *
 * 算法实现：
 * - 先画地图白色背景。
 * - 再按行列绘制网格线。
 */
void Renderer_DrawGrid(void) {
    if (!Renderer_IsReady()) {
        return;
    }

    wxRect boardRect(
        g_boardLeft,
        g_boardTop,
        g_boardPixelWidth,
        g_boardPixelHeight
    );

    g_memDC.SetBrush(wxBrush(COLOR_BOARD));
    g_memDC.SetPen(wxPen(wxColour(180, 185, 190), 2));
    g_memDC.DrawRectangle(boardRect);

    // 单元格过小时不画内部网格，避免画面过密
    if (g_cellSize >= 5) {
        g_memDC.SetPen(wxPen(COLOR_GRID, 1));

        for (int x = 0; x <= MAP_WIDTH; ++x) {
            int px = g_boardLeft + x * g_cellSize;
            g_memDC.DrawLine(
                px,
                g_boardTop,
                px,
                g_boardTop + g_boardPixelHeight
            );
        }

        for (int y = 0; y <= MAP_HEIGHT; ++y) {
            int py = g_boardTop + y * g_cellSize;
            g_memDC.DrawLine(
                g_boardLeft,
                py,
                g_boardLeft + g_boardPixelWidth,
                py
            );
        }
    }
}


/*
 * 绘制障碍物。
 *
 * 接口定义：
 * void Renderer_DrawObstacles(Map *map);
 *
 * 参数：
 * - map：地图对象
 *
 * 功能：
 * - 扫描 map->cells。
 * - 将 CELL_OBSTACLE 格子绘制为深色方块。
 *
 * 算法实现：
 * - 双重循环遍历 50x50 地图。
 * - 如果 cells[y][x] == CELL_OBSTACLE，则绘制障碍物。
 */
void Renderer_DrawObstacles(Map *map) {
    if (!Renderer_IsReady() || map == nullptr) {
        return;
    }

    for (int y = 0; y < MAP_HEIGHT; ++y) {
        for (int x = 0; x < MAP_WIDTH; ++x) {
            if (map->cells[y][x] == CELL_OBSTACLE) {
                Renderer_DrawCell(
                    Point(x, y),
                    COLOR_OBSTACLE,
                    wxColour(40, 40, 40)
                );
            }
        }
    }
}


/*
 * 绘制普通食物。
 *
 * 接口定义：
 * void Renderer_DrawFood(Map *map);
 *
 * 参数：
 * - map：地图对象
 *
 * 功能：
 * - 绘制普通食物。
 *
 * 算法实现：
 * - 优先扫描地图中的 CELL_FOOD。
 * - 如果没有扫描到，但 food_pos 合法，也绘制 food_pos。
 */
void Renderer_DrawFood(Map *map) {
    if (!Renderer_IsReady() || map == nullptr) {
        return;
    }

    bool hasDrawnFood = false;

    for (int y = 0; y < MAP_HEIGHT; ++y) {
        for (int x = 0; x < MAP_WIDTH; ++x) {
            if (map->cells[y][x] == CELL_FOOD) {
                Point p(x, y);
                wxRect rect = Renderer_CellRect(p);

                int padding = std::max(2, g_cellSize / 5);
                rect.Deflate(padding, padding);

                g_memDC.SetBrush(wxBrush(COLOR_FOOD));
                g_memDC.SetPen(wxPen(wxColour(160, 30, 30)));
                g_memDC.DrawEllipse(rect);

                hasDrawnFood = true;
            }
        }
    }

    if (!hasDrawnFood && Renderer_IsValidPoint(map->food_pos)) {
        wxRect rect = Renderer_CellRect(map->food_pos);

        int padding = std::max(2, g_cellSize / 5);
        rect.Deflate(padding, padding);

        g_memDC.SetBrush(wxBrush(COLOR_FOOD));
        g_memDC.SetPen(wxPen(wxColour(160, 30, 30)));
        g_memDC.DrawEllipse(rect);
    }
}


/*
 * 绘制一条蛇。
 *
 * 接口定义：
 * void Renderer_DrawSnake(Snake *snake, int player_id);
 *
 * 参数：
 * - snake：蛇对象
 * - player_id：玩家编号，用于决定颜色
 *
 * 功能：
 * - 绘制蛇身体。
 * - body[0] 为蛇头，使用更深颜色。
 *
 * 变量：
 * - bodyColor：蛇身颜色
 * - headColor：蛇头颜色
 *
 * 算法实现：
 * - 从蛇尾到蛇头绘制，保证蛇头显示在最上层。
 */
void Renderer_DrawSnake(Snake *snake, int player_id) {
    if (!Renderer_IsReady() || snake == nullptr) {
        return;
    }

    if (!snake->alive) {
        return;
    }

    wxColour bodyColor;
    wxColour headColor;

    if (player_id == 1) {
        bodyColor = COLOR_PLAYER1_BODY;
        headColor = COLOR_PLAYER1_HEAD;
    } else {
        bodyColor = COLOR_PLAYER2_BODY;
        headColor = COLOR_PLAYER2_HEAD;
    }

    int len = std::min(snake->length, MAX_SNAKE_LENGTH);

    // 先画身体，后画头部
    for (int i = len - 1; i >= 0; --i) {
        Point p = snake->body[i];

        if (!Renderer_IsValidPoint(p)) {
            continue;
        }

        bool isHead = (i == 0);

        Renderer_DrawCell(
            p,
            isHead ? headColor : bodyColor,
            wxColour(30, 80, 40)
        );

        // 蛇头画两个小眼睛，便于区分方向
        if (isHead && g_cellSize >= 8) {
            wxRect rect = Renderer_CellRect(p);

            int eyeSize = std::max(2, g_cellSize / 7);
            int offset = std::max(2, g_cellSize / 4);

            int cx = rect.x + rect.width / 2;
            int cy = rect.y + rect.height / 2;

            wxPoint eye1;
            wxPoint eye2;

            switch (snake->dir) {
            case DIR_UP:
                eye1 = wxPoint(cx - offset, cy - offset);
                eye2 = wxPoint(cx + offset - eyeSize, cy - offset);
                break;
            case DIR_DOWN:
                eye1 = wxPoint(cx - offset, cy + offset - eyeSize);
                eye2 = wxPoint(cx + offset - eyeSize, cy + offset - eyeSize);
                break;
            case DIR_LEFT:
                eye1 = wxPoint(cx - offset, cy - offset);
                eye2 = wxPoint(cx - offset, cy + offset - eyeSize);
                break;
            case DIR_RIGHT:
                eye1 = wxPoint(cx + offset - eyeSize, cy - offset);
                eye2 = wxPoint(cx + offset - eyeSize, cy + offset - eyeSize);
                break;
            default:
                eye1 = wxPoint(cx - offset, cy - offset);
                eye2 = wxPoint(cx + offset - eyeSize, cy - offset);
                break;
            }

            g_memDC.SetBrush(wxBrush(wxColour(255, 255, 255)));
            g_memDC.SetPen(*wxTRANSPARENT_PEN);
            g_memDC.DrawEllipse(eye1.x, eye1.y, eyeSize, eyeSize);
            g_memDC.DrawEllipse(eye2.x, eye2.y, eyeSize, eyeSize);
        }
    }
}


/*
 * 绘制道具。
 *
 * 接口定义：
 * void Renderer_DrawPowerUps(GameState *state);
 *
 * 参数：
 * - state：游戏状态
 *
 * 功能：
 * - 遍历 state->powerups。
 * - 对 active == true 的道具进行绘制。
 *
 * 算法实现：
 * - 道具绘制为彩色圆形。
 * - 圆形中显示一个汉字缩写。
 */
void Renderer_DrawPowerUps(GameState *state) {
    if (!Renderer_IsReady() || state == nullptr) {
        return;
    }

    for (int i = 0; i < POWERUP_ON_MAP; ++i) {
        PowerUp& power = state->powerups[i];

        if (!power.active || !Renderer_IsValidPoint(power.pos)) {
            continue;
        }

        wxRect rect = Renderer_CellRect(power.pos);

        int padding = std::max(1, g_cellSize / 8);
        rect.Deflate(padding, padding);

        wxColour color = Renderer_PowerUpColor(power.type);

        g_memDC.SetBrush(wxBrush(color));
        g_memDC.SetPen(wxPen(wxColour(80, 80, 80)));
        g_memDC.DrawEllipse(rect);

        if (g_cellSize >= 10) {
            g_memDC.SetFont(wxFont(
                std::max(7, g_cellSize / 2),
                wxFONTFAMILY_DEFAULT,
                wxFONTSTYLE_NORMAL,
                wxFONTWEIGHT_BOLD
            ));

            Renderer_DrawCenteredText(
                Renderer_PowerUpLabel(power.type),
                rect,
                wxColour(255, 255, 255)
            );

            g_memDC.SetFont(wxFont(
                11,
                wxFONTFAMILY_DEFAULT,
                wxFONTSTYLE_NORMAL,
                wxFONTWEIGHT_NORMAL
            ));
        }
    }
}


/*
 * 绘制 UI。
 *
 * 接口定义：
 * void Renderer_DrawUI(GameState *state);
 *
 * 参数：
 * - state：游戏状态
 *
 * 功能：
 * - 显示玩家分数。
 * - 显示当前模式。
 * - 显示道具状态。
 *
 * 算法实现：
 * - UI 固定绘制在地图下方。
 * - 根据 has_snake2 判断是否显示玩家2信息。
 */
void Renderer_DrawUI(GameState *state) {
    if (!Renderer_IsReady() || state == nullptr) {
        return;
    }

    g_memDC.SetPen(wxPen(wxColour(210, 215, 220)));
    g_memDC.DrawLine(0, g_uiTop, g_windowWidth, g_uiTop);

    g_memDC.SetTextForeground(COLOR_TEXT);

    g_memDC.SetFont(wxFont(
        12,
        wxFONTFAMILY_DEFAULT,
        wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_BOLD
    ));

    wxString line1 = wxString::Format(
        wxString::FromUTF8("玩家1 分数：%d    模式：%s    步数：%d"),
        state->snake1.score,
        Renderer_GameModeName(state->mode),
        state->step_count
    );

    g_memDC.DrawText(line1, BOARD_MARGIN, g_uiTop + 10);

    if (state->has_snake2) {
        wxString line2 = wxString::Format(
            wxString::FromUTF8("玩家2 分数：%d    类型：%s"),
            state->snake2.score,
            state->snake2_is_ai
                ? wxString::FromUTF8("AI")
                : wxString::FromUTF8("玩家")
        );

        g_memDC.DrawText(line2, BOARD_MARGIN, g_uiTop + 34);
    }

    g_memDC.SetFont(wxFont(
        10,
        wxFONTFAMILY_DEFAULT,
        wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_NORMAL
    ));

    wxString powerText = wxString::Format(
        wxString::FromUTF8("玩家1道具状态：磁铁剩余 %d 个食物，破壁钻头剩余 %d 个食物，减速剩余 %d 个食物，移动间隔 %d ms"),
        state->snake1.magnet_food_left,
        state->snake1.drill_food_left,
        state->snake1.slow_food_left,
        state->snake1.move_interval_ms
    );

    g_memDC.DrawText(powerText, BOARD_MARGIN, g_uiTop + 60);
}


/*
 * 绘制游戏结束界面。
 *
 * 接口定义：
 * void Renderer_DrawGameOver(GameState *state);
 *
 * 参数：
 * - state：游戏状态
 *
 * 功能：
 * - 在地图中央显示游戏结束信息。
 * - 显示最终得分。
 *
 * 算法实现：
 * - 绘制一个居中的白色信息框。
 * - 根据双人模式判断是否显示玩家2分数。
 */
void Renderer_DrawGameOver(GameState *state) {
    if (!Renderer_IsReady() || state == nullptr) {
        return;
    }

    wxRect box(
        g_boardLeft + g_boardPixelWidth / 2 - 170,
        g_boardTop + g_boardPixelHeight / 2 - 90,
        340,
        180
    );

    g_memDC.SetBrush(wxBrush(wxColour(255, 255, 255)));
    g_memDC.SetPen(wxPen(wxColour(60, 60, 60), 2));
    g_memDC.DrawRoundedRectangle(box, 12);

    g_memDC.SetFont(wxFont(
        22,
        wxFONTFAMILY_DEFAULT,
        wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_BOLD
    ));

    wxRect titleRect(box.x, box.y + 20, box.width, 40);
    Renderer_DrawCenteredText(
        wxString::FromUTF8("游戏结束"),
        titleRect,
        wxColour(200, 50, 50)
    );

    g_memDC.SetFont(wxFont(
        13,
        wxFONTFAMILY_DEFAULT,
        wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_NORMAL
    ));

    wxString score1 = wxString::Format(
        wxString::FromUTF8("玩家1最终得分：%d"),
        state->snake1.score
    );

    wxRect scoreRect1(box.x, box.y + 75, box.width, 30);
    Renderer_DrawCenteredText(score1, scoreRect1, COLOR_TEXT);

    if (state->has_snake2) {
        wxString score2 = wxString::Format(
            wxString::FromUTF8("玩家2最终得分：%d"),
            state->snake2.score
        );

        wxRect scoreRect2(box.x, box.y + 105, box.width, 30);
        Renderer_DrawCenteredText(score2, scoreRect2, COLOR_TEXT);
    }

    g_memDC.SetFont(wxFont(
        10,
        wxFONTFAMILY_DEFAULT,
        wxFONTSTYLE_NORMAL,
        wxFONTWEIGHT_NORMAL
    ));

    wxRect hintRect(box.x, box.y + 140, box.width, 25);
    Renderer_DrawCenteredText(
        wxString::FromUTF8("按 R 重新开始，按 ESC 退出"),
        hintRect,
        wxColour(100, 100, 100)
    );
}


/*
 * 刷新屏幕。
 *
 * 接口定义：
 * void Renderer_Present(void);
 *
 * 功能：
 * - 将双缓冲画面提交到窗口显示。
 *
 * 算法实现：
 * - Refresh(false)：请求重绘，不清空背景。
 * - Update()：立即处理重绘事件。
 * - Yield(true)：让 wxWidgets 处理窗口事件，避免窗口无响应。
 */
void Renderer_Present(void) {
    if (!Renderer_IsReady()) {
        return;
    }

    g_canvas->Refresh(false);
    g_canvas->Update();

    if (wxTheApp) {
        wxTheApp->Yield(true);
    }
}


/*
 * 关闭图形系统。
 *
 * 接口定义：
 * void Renderer_Destroy(void);
 *
 * 功能：
 * - 释放双缓冲资源。
 * - 关闭窗口。
 * - 如果 wxWidgets 是由 Renderer_Init 启动的，则清理 wxWidgets。
 *
 * 算法实现：
 * 1. 将 wxMemoryDC 与 wxBitmap 解绑。
 * 2. 销毁窗口。
 * 3. 清理 wxWidgets。
 */
void Renderer_Destroy(void) {
    if (g_memDC.IsOk()) {
        g_memDC.SelectObject(wxNullBitmap);
    }

    if (g_frame != nullptr) {
        g_frame->Destroy();
        g_frame = nullptr;
        g_canvas = nullptr;
    }

    g_backBuffer = wxBitmap();

    if (g_wxStartedByRenderer) {
        wxEntryCleanup();
        g_wxStartedByRenderer = false;
    }
}