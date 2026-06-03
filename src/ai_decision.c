#include "ai.h"
#include <string.h>

Action ai_decide_action(GameState* gs, int ai_idx) {
    Action act = {1, -1, -1};  // 默认结束回合
    if (gs == NULL || ai_idx < 0 || ai_idx >= gs->player_count) return act;
    Character* ai = &gs->players[ai_idx];
    if (ai->hand_count <= 0) return act;

    act.action_type = 0;
    act.card_index = 0;
    act.target = 0;
    if (ai->personality == PERSON_RADICAL) {
        return act;
    }
    if (ai->personality == PERSON_CONSERVATIVE) {
        return act;
    }
    if (ai->personality == PERSON_GAMBLER) {
        return act;
    }
    return act;
}

int ai_should_use_tao_to_save(GameState* gs, int saver_idx, int dying_idx) {
    (void)gs;
    (void)saver_idx;
    (void)dying_idx;
    return 0;
}

const char* ai_get_last_psych_message() { return "AI思考中..."; }
int ai_get_last_emotion() { return 0; }
