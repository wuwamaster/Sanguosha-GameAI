#include "ai.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char g_last_psych_message[128] = "AI思考中...";
static int g_last_emotion = 0;

const char* ai_get_last_psych_message() {
    return g_last_psych_message;
}

int ai_get_last_emotion() {
    return g_last_emotion;
}

void ai_generate_psych_message(GameState* gs, int ai_idx, Action act) {
    if (gs == NULL || ai_idx < 0 || ai_idx >= gs->player_count) {
        strncpy(g_last_psych_message, "AI思考中...", sizeof(g_last_psych_message));
        g_last_emotion = 0;
        return;
    }

    Character* ai = &gs->players[ai_idx];
    Character* target = NULL;
    if (act.target >= 0 && act.target < gs->player_count) {
        target = &gs->players[act.target];
    }

    const char* message = "AI沉思中...";
    int emotion = 0;

    if (ai->personality == PERSON_RADICAL) {
        emotion = 2;
        if (act.action_type == 1) {
            message = "暂时撤退，下一回合再发起猛攻。";
        } else {
            switch (ai->hand[act.card_index].type) {
                case CARD_SHA:
                    if (target) {
                        char buffer[128];
                        snprintf(buffer, sizeof(buffer), "这一刀砍过去！目标剩余%d血。", target->hp);
                        strncpy(g_last_psych_message, buffer, sizeof(g_last_psych_message));
                        g_last_emotion = emotion;
                        return;
                    }
                    message = "出杀！不给对手喘息机会。";
                    break;
                case CARD_TAO:
                    message = "血量危险，先补一口桃。";
                    break;
                case CARD_GUO_CAI:
                    message = "拆你手牌，别给我反击机会。";
                    break;
                case CARD_WU_ZHONG:
                    message = "再摸两张牌，准备下一波攻击。";
                    break;
                default:
                    message = "先稳住，不要轻易浪费手牌。";
                    break;
            }
        }
    } else if (ai->personality == PERSON_CONSERVATIVE) {
        emotion = 3;
        if (act.action_type == 1) {
            message = "稳住阵脚，别急着出手。";
        } else {
            switch (ai->hand[act.card_index].type) {
                case CARD_SHA:
                    message = "先试探一下，看他有没有闪。";
                    break;
                case CARD_TAO:
                    message = "小心为上，先补血。";
                    break;
                case CARD_GUO_CAI:
                    message = "拆掉一个潜在威胁。";
                    break;
                case CARD_WU_ZHONG:
                    message = "先补牌，再观察局势。";
                    break;
                default:
                    message = "先保留这张牌，留到更稳妥的时候再用。";
                    break;
            }
        }
    } else if (ai->personality == PERSON_GAMBLER) {
        emotion = 4;
        if (act.action_type == 1) {
            message = "这把先不急，赌一把再说。";
        } else {
            switch (ai->hand[act.card_index].type) {
                case CARD_SHA:
                    message = "赌你没有闪，干！";
                    break;
                case CARD_TAO:
                    message = "运气来了，先补个桃。";
                    break;
                case CARD_GUO_CAI:
                    message = "拆掉你的手牌，看看运气如何。";
                    break;
                case CARD_WU_ZHONG:
                    message = "摸两张，赌个好牌！";
                    break;
                default:
                    message = "随性出牌，看看会不会有惊喜。";
                    break;
            }
        }
    }

    strncpy(g_last_psych_message, message, sizeof(g_last_psych_message));
    g_last_psych_message[sizeof(g_last_psych_message) - 1] = '\0';
    g_last_emotion = emotion;
}
