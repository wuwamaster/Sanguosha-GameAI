# 三国杀·人格分裂AI

> 极简三国杀 C 语言实现，AI 具有三种人格（激进派、保守派、赌徒派），可实时显示心理活动与表情。

## 功能特点

- 两种游戏模式：单挑 / 非对称主公局
- 3 名武将（张飞、赵云、诸葛亮）+ 5 种基本卡牌
- AI 人格分裂：每局随机人格，决策加权 + 随机扰动
- 动态心理文字：AI 解释每一步决策
- 表情交互：根据情绪显示表情反馈
- 文件统计：对局记录保存与胜率查询
- raylib 图形界面，可切换控制台版 UI 进行调试与测试

## 环境要求

- Windows 7/10/11
- MinGW-w64 或 Visual Studio
- 图形版依赖项目内自带的 `vendor/raylib/`，无需额外安装 EasyX

## UI 实现

项目包含两套实现同一接口 `include/ui.h` 的 UI：

- `src/ui_raylib.c`
  图形版 UI，供主程序运行使用。提供窗口、鼠标点击、主菜单、手牌区、AI 状态区、最近出牌区和 AI 心理气泡。
- `src/ui_console.c`
  控制台版 UI，供调试与测试使用。通过终端打印游戏状态，使用键盘输入数字进行操作。

当前 `Makefile` 的默认行为：

- `make` 生成主程序 `sanguosha.exe`，使用 `ui_raylib.c`
- `make test` 生成 `tests/test_*.exe`，使用 `ui_console.c`

若修改了 `ui.h` 中的接口或 UI 行为，两份实现都应同步更新。

## UI 设计简要说明

图形版 UI 基于 raylib，整体布局如下：

- 顶部区域：显示 AI 武将、人格、血量和手牌数量
- 中部区域：显示牌堆信息、当前回合、阶段、最近出牌记录和 AI 心理文字
- 底部区域：显示玩家武将、血条、手牌和操作按钮
- 主菜单：通过鼠标完成模式、阵营和武将选择

控制台版 UI 采用纯文本交互，适合：

- 没有图形环境时快速试运行
- 跑测试时避免依赖图形窗口
- 调试回合逻辑、AI 决策和输入输出流程

## 编译与运行

### 1. 图形版运行方式

在项目根目录执行：

```bash
make clean
make
./sanguosha.exe
```

说明：

- 使用 `src/ui_raylib.c`
- 需要从项目根目录启动，确保能读取 `assets/fonts/simhei.ttf`
- 启动后通过鼠标完成菜单选择、出牌和目标选择

### 2. 控制台版运行方式

如果需要直接用控制台方式运行主程序，可手动编译：

```bash
gcc -Wall -g -I./include \
    src/main.c src/game_engine.c src/ai_decision.c src/file_io.c \
    src/skills.c src/ai_psych.c src/cards.c src/stats.c src/ui_console.c \
    -o sanguosha_console.exe

./sanguosha_console.exe
```

说明：

- 使用 `src/ui_console.c`
- 通过键盘输入数字进行操作
- 如果控制台中文乱码，可先执行 `chcp 65001`

### 3. 测试运行方式

```bash
make test
./tests/test_engine.exe
```

说明：

- 测试程序默认链接 `ui_console.c`
- 这样可以避免测试阶段依赖 raylib 图形窗口

## 运行提示

- 图形版：先选模式，再选阵营（主公局），再选武将，之后点击手牌和目标
- 控制台版：按提示输入数字，`1` 表示出牌，`2` 表示结束回合
- 若要切换主程序默认 UI，可修改 `Makefile` 中 `MAIN_SRC` 的排除项
