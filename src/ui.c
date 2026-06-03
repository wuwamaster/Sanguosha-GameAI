#include "ui.h"
#include <stdio.h>
#include <stdlib.h>

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
void ui_init() {
    system("cls");
    system("title 三国杀·人格分裂AI - 控制台版");
    printf("========== 欢迎来到 三国杀·人格分裂AI ==========\n");
    printf("提示：输入数字进行操作，按回车确认\n\n");
}

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
    printf(">>> 当前回合: %s <<<\n",
           gs->current_turn == 0 ? "玩家" : "AI");
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

// 关闭UI（控制台版无需特殊处理）
void ui_close() {
    printf("谢谢游玩！\n");
}