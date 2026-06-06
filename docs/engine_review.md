# Engine 实现评审：需要修正的问题

这是针对你组员提供的 `teampull/src/game_engine.c` 的评审结果。该实现比当前仓库骨架更完整，但还存在若干问题，建议按照下面清单逐条修改。

## 1. 头文件与结构体定义不一致

当前代码使用了以下 `GameState` 字段，但你们仓库里的 `include/game_engine.h` 尚未定义它们：

- `turn_phase`
- `sha_used_this_turn`
- `need_shan_response`
- `need_discard`
- `shan_source`
- `shan_target`

这说明你们的 `GameState` 结构体已经需要扩展。请务必先将这些字段加入 `include/game_engine.h`，否则编译会失败或产生内存布局错误。

## 2. `save_dying` 中应该跳过已死亡角色

`save_dying` 逐个检查救人角色手牌是否含桃，但没有判断救人者是否存活：

```c
Character* saver = &gs->players[searcher];
for (int i = 0; i < saver->hand_count; i++) {
```

如果 `searcher` 已经血量为 0 以下，代码仍然会让该角色参与救援判断。应加上 `saver->hp > 0` 的判断。

## 3. `game_get_legal_actions` 的目标筛选不够准确

问题如下：

- `CARD_SHA` 只为每张杀找到一个目标，就跳出内部循环；这会漏掉其它可攻击目标。
- `CARD_GUO_CAI` 的目标合法性只判断 `hp > 0`，没有排除对自己使用。
- `CARD_WU_ZHONG` 的目标规则目前允许对所有存活目标，不够严谨。

建议：

- 对每张手牌列出所有合法目标，而不是找到一个就停止。
- 明确 `GUO_CAI` 只能对其他角色使用。
- `WU_ZHONG` 应该只对自己使用，或严格按设计定义。

## 4. `game_perform_action` 与 `game_resolve_shan` 的流程关系不明确

`game_perform_action` 对 `CARD_SHA` 仅设置 `need_shan_response` 并返回成功，真正的伤害逻辑由 `game_resolve_shan` 处理。

这没错，但必须明确：

- 谁来调用 `game_resolve_shan`？
- 何时清理 `need_shan_response` 和 `shan_source` / `shan_target`？

否则会出现“回合停在等待闪响应，却无人继续处理”的状态。

## 5. `game_check_death` 中胜负判定逻辑混乱

其中几处逻辑有冗余和可读性问题：

- `gs->winner = gs->players[gs->current_turn].camp == CAMP_PLAYER ? 0 : 1;` 这一句在“反贼全死”分支中并不必要，且后面又被覆盖。
- `gs->winner = (gs->players[idx].camp == CAMP_PLAYER) ? 1 : 0;` 这一赋值在单挑模式下反向了一次，虽然意图是敌方胜利，但写法容易让人误解。

建议重构为：

- 单挑：玩家对AI，若玩家死亡则 `winner=1`，否则 `winner=0`。
- 主公局：先判断主公是否死亡，再判断所有反贼是否死亡。

## 6. `game_init` 中主公逻辑写法不够稳健

当前实现只在 `MODE_LORD_VS_REBELS` 下把 `players[1]` 设为 AI 主公：

```c
if (!player_is_lord) {
    gs->players[1].is_lord = 1;
    gs->players[1].max_hp = 5;
    gs->players[1].hp = 5;
}
```

这会假设 `players[1]` 一定是主公，`players[2]` 一定是反贼。建议明确注释或改成更通用的分配方式。

## 7. `draw_card` 牌堆耗尽时的应急处理存在问题

当抽牌堆和弃牌堆都为空时，直接发一张 `CARD_SHA`：

```c
ch->hand[ch->hand_count++] = (Card){CARD_SHA, 0};
```

这会让玩家在牌堆耗尽时突然获得一张默认杀，建议改成“什么也不发”或引入更明确的空牌策略。

## 8. `game_shuffle_deck`、`srand` 与随机数初始化

`game_init` 每次调用都会执行 `srand((unsigned)time(NULL));`。如果游戏初始化非常快，可能会产生相同随机序列。

建议把随机种子初始化放在程序启动时只调用一次，而不是每次 `game_init` 都调用。

## 9. 编码风格与可维护性建议

- `game_check_death` 中如果 `gs->players[idx].hp <= 0` 已经死亡，应该先判断 `is_lord`、`camp`，然后再调用 `save_dying`。
- `game_next_turn` 里对 AI 弃牌处理的循环会把手牌从头部连续删除，虽然可行，但最好加注释说明“AI 简化丢弃策略”。
- 文件顶部应补 `#include <time.h>`，因为 `time(NULL)` 在此文件中使用，但该文件内没有包含 `time.h`。

## 10. 建议你要他重点修改的几件事

1. 先把 `include/game_engine.h` 的 `GameState` 扩展成和他代码一致。
2. 修正 `save_dying`，跳过 `hp <= 0` 的救援角色。
3. 修正 `game_get_legal_actions` 的目标判断逻辑。
4. 清理 `game_check_death` 中不必要的 winner 赋值，并确保胜负判定清晰。
5. 说明 `game_resolve_shan` 的调用时机，并写注释或设计说明。

---

如果你愿意，我也可以继续把这份评审转换为“你直接发给组员的反馈邮件/回复文本”。