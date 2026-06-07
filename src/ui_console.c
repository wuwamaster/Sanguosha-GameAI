#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
  #include <windows.h>
#endif

void ui_init() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif
    system("cls");
    system("title 三国杀·人格分裂AI - 控制台版");
    printf("========== 欢迎来到 三国杀·人格分裂AI ==========\n");
    printf("提示：输入数字进行操作，按回车确认\n\n");
}

// 辅助函数：根据武将类型返回名字
static const char* hero_name(HeroType hero) {
    switch (hero) {
        case HERO_ZHANG_FEI:   return "张飞";
        case HERO_ZHAO_YUN:    return "赵云";
        case HERO_ZHU_GE_LIANG:return "诸葛亮";
        default: return "未知";
    }
}

// 辅助函数：根据卡牌类型返回名字
static const char* card_name(CardType type) {
    switch (type) {
        case CARD_SHA:      return "杀";
        case CARD_SHAN:     return "闪";
        case CARD_TAO:      return "桃";
        case CARD_GUO_CAI:  return "过河拆桥";
        case CARD_WU_ZHONG: return "无中生有";
        default: return "?";
    }
}

static const char* phase_name(int phase) {
    switch (phase) {
        case 0: return "准备阶段";
        case 1: return "摸牌阶段";
        case 2: return "出牌阶段";
        case 3: return "弃牌阶段";
        case 4: return "回合结束";
        default: return "未知";
    }
}

// 辅助函数：根据表情ID返回 Emoji 字符串
static const char* emotion_to_emoji(int emotion_id) {
    switch (emotion_id) {
        case 1: return "😠";   // 愤怒
        case 2: return "😊";   // 得意
        case 3: return "😏";   // 奸笑
        case 4: return "😨";   // 惊恐
        default: return "😐";
    }
}

// 初始化（控制台版：清屏，设置控制台标题）

// 绘制完整游戏界面
void ui_draw_game(GameState* gs) {
    system("cls");
    printf("========== 当前游戏状态 ==========\n\n");

    // 显示玩家信息
    Character* player = &gs->players[0];
    printf("【玩家】 %s\n", hero_name(player->hero));
    printf("  血量: %d / %d", player->hp, player->max_hp);
    // 简单的血条显示
    printf("  [");
    int i;
    for (i = 0; i < player->hp; i++) printf("█");
    for (; i < player->max_hp; i++) printf("░");
    printf("]\n");
    printf("  手牌 (%d张): ", player->hand_count);
    for (i = 0; i < player->hand_count; i++) {
        printf("%d:%s ", i, card_name(player->hand[i].type));
    }
    printf("\n\n");

    // 显示所有AI（支持最多2个）
    for (int idx = 1; idx < gs->player_count; idx++) {
        Character* ai = &gs->players[idx];
        printf("【AI%d】 %s (人格: %s)\n", idx, hero_name(ai->hero),
               ai->personality == PERSON_RADICAL ? "激进派" :
               (ai->personality == PERSON_CONSERVATIVE ? "保守派" : "赌徒派"));
        printf("  血量: %d / %d", ai->hp, ai->max_hp);
        printf("  [");
        for (i = 0; i < ai->hp; i++) printf("█");
        for (; i < ai->max_hp; i++) printf("░");
        printf("]\n");
        printf("  手牌数: %d\n\n", ai->hand_count);
    }

    // 显示当前回合
    printf(">>> 当前回合: %s   [%s] <<<\n",
           gs->current_turn == 0 ? "玩家" : "AI",
           phase_name(gs->turn_phase));
    printf("===================================\n");
}

// 获取玩家行动（通过键盘输入）
Action ui_get_player_action(GameState* gs) {
    Action act;
    int choice, card_idx, target;

    while (1) {
        printf("\n请选择操作：\n");
        printf("  1. 出牌\n");
        printf("  2. 结束回合\n");
        printf("请输入数字: ");
        if (scanf("%d", &choice) != 1) {
            // 清除错误输入
            while (getchar() != '\n');
            printf("输入无效，请重新输入！\n");
            continue;
        }

        if (choice == 2) {
            act.action_type = 1;   // 结束回合
            act.card_index = -1;
            act.target = -1;
            return act;
        }
        else if (choice == 1) {
            // 出牌：需要选择手牌和目标
            printf("当前手牌：");
            Character* player = &gs->players[0];
            for (int i = 0; i < player->hand_count; i++) {
                printf("%d:%s ", i, card_name(player->hand[i].type));
            }
            printf("\n请选择要出的牌（输入编号）: ");
            if (scanf("%d", &card_idx) != 1 || card_idx < 0 || card_idx >= player->hand_count) {
                printf("手牌编号无效，请重新选择！\n");
                while (getchar() != '\n');
                continue;
            }

            // 选择目标（仅当卡牌需要目标时，简单起见让玩家总是选目标）
            printf("请选择目标：\n");
            printf("  0 - 自己\n");
            for (int i = 1; i < gs->player_count; i++) {
                printf("  %d - AI%d\n", i, i);
            }
            printf("请输入目标编号: ");
            if (scanf("%d", &target) != 1 || target < 0 || target >= gs->player_count) {
                printf("目标无效，请重新选择！\n");
                while (getchar() != '\n');
                continue;
            }

            act.action_type = 0;
            act.card_index = card_idx;
            act.target = target;
            return act;
        }
        else {
            printf("无效选择，请输入 1 或 2。\n");
        }
    }
}

// 显示心理活动文字和表情（控制台版用 Emoji）
void ui_show_psych_message(const char* message, int emotion_id) {
    printf("\n[AI心理] %s %s\n", emotion_to_emoji(emotion_id), message);
}

// 游戏结束画面
void ui_show_gameover(int winner) {
    printf("\n========== 游戏结束 ==========\n");
    if (winner == 0) {
        printf("         【玩家胜利！】\n");
    } else {
        printf("         【AI胜利！】\n");
    }
    printf("===============================\n");
    printf("按任意键退出...");
    getchar();
    getchar();  // 吸收前面的回车
}

int ui_get_shan_response(GameState* gs) {
    Character* player = &gs->players[0];
    printf("\n>>> 你被【杀】了！请选择是否出【闪】：\n");
    int has_shan = 0;
    for (int i = 0; i < player->hand_count; i++) {
        if (player->hand[i].type == CARD_SHAN) {
            printf("  %d - 闪\n", i);
            has_shan = 1;
        }
    }
    if (!has_shan) {
        printf("  (你没有闪，自动承受伤害)\n");
        return -1;
    }
    printf("  -1 - 不出闪（承受伤害）\n");
    printf("请输入手牌编号: ");
    int idx;
    if (scanf("%d", &idx) != 1) {
        while (getchar() != '\n');
        return -1;
    }
    if (idx >= 0 && idx < player->hand_count && player->hand[idx].type == CARD_SHAN)
        return idx;
    return -1;
}

int ui_get_discard_choice(GameState* gs, int remaining) {
    Character* player = &gs->players[0];
    printf("\n>>> 弃牌阶段：手牌数(%d)超过体力(%d)，需弃置 %d 张\n",
           player->hand_count, player->hp, remaining);
    printf("当前手牌：");
    for (int i = 0; i < player->hand_count; i++)
        printf("%d:%s ", i, card_name(player->hand[i].type));
    printf("\n请选择要弃置的牌（输入编号）: ");
    int idx;
    if (scanf("%d", &idx) != 1) {
        while (getchar() != '\n');
        return -1;
    }
    if (idx < 0 || idx >= player->hand_count) return -1;
    return idx;
}

// 关闭UI（控制台版无需特殊处理）
void ui_close() {
    printf("谢谢游玩！\n");
}

// 节奏控制（控制台版：用 stdio 节奏 fflush 即可，无需真延迟）
void ui_pause(GameState* gs, double seconds) {
    (void)gs; (void)seconds;
    /* 测试不卡时间 */
}

// 出牌通知（控制台版只打印一行）
void ui_show_card_played(int actor_idx, Card card, int target_idx) {
    const char* card_name_local;
    switch (card.type) {
        case CARD_SHA:      card_name_local = "杀"; break;
        case CARD_SHAN:     card_name_local = "闪"; break;
        case CARD_TAO:      card_name_local = "桃"; break;
        case CARD_GUO_CAI:  card_name_local = "过河拆桥"; break;
        case CARD_WU_ZHONG: card_name_local = "无中生有"; break;
        default:            card_name_local = "?"; break;
    }
    if (target_idx >= 0)
        printf("[出牌] 角色%d 使用 %s -> 角色%d\n", actor_idx, card_name_local, target_idx);
    else
        printf("[出牌] 角色%d 使用 %s\n", actor_idx, card_name_local);
}

// 主菜单（控制台版）：用于测试与 fallback。返回 1 确认。
int ui_show_menu(MenuChoice* out) {
    if (!out) return 0;
    int mode = 0, hero = 0, lord = 0;
    printf("\n=== 主菜单 ===\n");
    printf("模式 (0=单挑, 1=主公局): "); if (scanf("%d", &mode) != 1) mode = 0;
    printf("武将 (0=张飞, 1=赵云, 2=诸葛亮): "); if (scanf("%d", &hero) != 1) hero = 0;
    if (mode == 1) {
        printf("阵营 (0=主公, 1=反贼): "); if (scanf("%d", &lord) != 1) lord = 0;
        lord = (lord == 0) ? 1 : 0;
    }
    out->mode = (mode == 1) ? MODE_LORD_VS_REBELS : MODE_SINGLE;
    out->player_hero = (HeroType)hero;
    out->player_is_lord = lord;
    out->ai_count = (mode == 1) ? 2 : 1;
    /* 简单填充：避免与玩家同武将 */
    HeroType pool[3] = {HERO_ZHANG_FEI, HERO_ZHAO_YUN, HERO_ZHU_GE_LIANG};
    int j = 0;
    for (int i = 0; i < 3 && j < out->ai_count; i++)
        if (pool[i] != out->player_hero) out->ai_hero[j++] = pool[i];
    out->ai_person[0] = PERSON_RADICAL;
    out->ai_person[1] = PERSON_CONSERVATIVE;
    return 1;
}

void ui_get_star_choice(GameState* gs) {
    if (gs == NULL || !gs->need_star_choice) return;

    printf("\n========== 观星 ==========\n");
    printf("牌堆顶 %d 张，按当前排列顺序（从左到右 = 牌堆顶到底部）：\n", gs->star_watch_count);
    printf("  ");
    for (int i = 0; i < gs->star_watch_count; i++) {
        int card_idx = gs->star_current_slots[i];
        printf("[%d] %s    ", i + 1, card_name(gs->star_watch_cards[card_idx].type));
    }
    printf("\n");
    printf("命令：输入两个槽位号交换（如 1 3），或输入 0 确认\n");

    while (1) {
        printf("> ");
        int a, b;
        if (scanf("%d", &a) != 1) break;
        if (a == 0) break;
        if (scanf("%d", &b) != 1) break;
        a--; b--;
        if (a >= 0 && a < gs->star_watch_count && b >= 0 && b < gs->star_watch_count) {
            game_swap_star_slots(gs, a, b);
            printf("  交换后：");
            for (int i = 0; i < gs->star_watch_count; i++) {
                int ci = gs->star_current_slots[i];
                printf("[%d] %s  ", i + 1, card_name(gs->star_watch_cards[ci].type));
            }
            printf("\n");
        } else {
            printf("  无效槽位\n");
        }
    }
    printf("=========================\n\n");
}