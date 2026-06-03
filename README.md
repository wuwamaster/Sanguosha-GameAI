# 三国杀·人格分裂AI

> 极简三国杀C语言实现，AI具有三种人格（激进派、保守派、赌徒派），可实时显示心理活动与表情。

## 功能特点
- 🎮 两种游戏模式：单挑 / 非对称主公局
- 🃏 3名武将（张飞、赵云、诸葛亮）+ 5种基本卡牌
- 🤖 AI人格分裂：每局随机人格，决策加权 + 随机扰动
- 💬 动态心理文字：AI解释每一步决策
- 😊 表情交互：根据情绪显示表情图标
- 🗂️ 文件统计：对局记录保存与胜率查询
- 🖼️ EasyX图形界面（可切换控制台调试版）

## 快速开始

### 环境要求
- Windows 7/10/11
- MinGW-w64 或 Visual Studio
- EasyX 图形库（[下载](https://easyx.cn)）

### 编译与运行
```bash
git clone https://github.com/qdooo-w/Sanguosha-GameAI.git
cd Sanguosha-GameAI
make
./sanguosha.exe
