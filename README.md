# Snake Battle — Exquisite Edition

基于 C++ + raylib 的贪吃蛇对战游戏，支持四种模式、五种道具、AI 对战。

---

## 项目结构

```
snake/
├── common.h          # 公共数据结构、枚举、常量
├── game_core.h/cpp   # 核心逻辑：蛇移动、碰撞、死亡判定
├── map.h/cpp         # 地图系统：障碍物、食物生成
├── powerup.h/cpp     # 道具系统：5 种道具 + 效果管理
├── ai.h/cpp          # AI 对手：寻路、避障决策
├── input.h/cpp       # 键盘输入：WASD / 方向键
├── renderer.h/cpp    # 图形渲染引擎（raylib）
├── main.cpp          # 主循环 + 菜单系统
├── build.sh          # 一键编译脚本
└── README.md
```

## 游戏模式

| 模式 | 说明 |
|------|------|
| **BASIC** | 经典贪吃蛇，50×50 地图，20 个障碍物，随机事件增加障碍 |
| **POWER-UP** | 在基础版上增加道具系统，地图维持 2 个道具 |
| **PvP BATTLE** | 双人对战：P1 用 WASD，P2 用方向键 |
| **VS AI** | 人机对战：AI 自动寻路避障 |

## 操作说明

| 按键 | 功能 |
|------|------|
| **W A S D** | 玩家 1 方向控制（单人模式也支持方向键） |
| **方向键** | 玩家 2 方向控制（仅双人对战） |
| **P** | 暂停游戏 |
| **ESC** | 暂停时恢复 / 菜单退出 |
| **Q** | 退出 |
| **↑ ↓ Enter** | 菜单 / 弹窗选择 |

## 游戏规则

- **地图**：50×50，带边界，撞墙死亡
- **初始蛇长**：3 节，每秒移动 9 格
- **食物**：每吃一个 +1 分，长度 +1 节
- **障碍物**：初始 20 个，每 100~300 步随机新增 1~3 个
- **死亡条件**：撞墙 / 撞障碍物 / 撞自己 / 蛇头撞对方蛇身
- **对战规则**：蛇头撞蛇身 → 攻击方死；蛇头相撞 → 双死

## 五种道具

| 道具 | 效果 |
|------|------|
| **超大食物** (10) | +10 分，长度 +1 |
| **磁铁** (M) | 吸附蛇头 3×3 范围内的食物，持续吃 2 个食物 |
| **破壁钻头** (D) | 可穿过障碍物，持续吃 2 个食物 |
| **消食片** (-) | 分数不变，蛇长变为 2/3（最短 3 节） |
| **减速** (S) | 移动速度减半，持续吃 2 个食物 |

## 编译 & 运行

### 依赖

- MinGW-w64 (GCC，UCRT 运行时)
- [raylib](https://www.raylib.com/) 6.0

### 一键编译

```bash
cd snake
./build.sh
```

### 手动编译

```bash
export PATH="/c/Users/lenovo/mingw64/mingw64/bin:$PATH"

# 先编译 raylib（仅需一次）
cd raylib-6.0/src
mingw32-make PLATFORM=PLATFORM_DESKTOP_WIN32 \
  CFLAGS="-D_WIN32_WINNT=0x0A00 -DNTDDI_VERSION=0x0A000000 -DPLATFORM_DESKTOP_WIN32 -DGRAPHICS_API_OPENGL_33 -Wno-missing-braces -Werror=pointer-arith -fno-strict-aliasing -std=c99" -j4

# 编译游戏
cd snake
g++ -std=c++17 \
  -I../raylib-6.0/src \
  -D_WIN32_WINNT=0x0A00 \
  -o snake_battle.exe \
  main.cpp ai.cpp game_core.cpp map.cpp powerup.cpp input.cpp renderer.cpp \
  ../raylib-6.0/src/libraylib.a \
  -lopengl32 -lgdi32 -lwinmm \
  -static-libgcc -static-libstdc++
```

生成的 `snake_battle.exe` 为独立可执行文件，无需 DLL。

## 图形特性

- 深色赛博朋克主题 + 细线网格
- 渐变蛇身（头亮尾暗）+ 圆角段片 + 眼睛
- 食物脉冲光晕动画
- 五种道具独立配色 + 菱形图标
- 玻璃拟态侧边栏 UI
- 精美的菜单 / 暂停 / 结算弹窗
