#include "../include/game_engine.h"
#include "../include/ai.h"
#include "test_utils.h"
#include <string.h>

static void init_state(GameState* gs) {
    memset(gs, 0, sizeof(GameState));
    gs->player_count = 2;
    gs->current_turn = 1;
    gs->game_over = 0;
    gs->winner = 0;
    gs->players[0].hero = HERO_ZHANG_FEI;
    gs->players[0].hp = 3;
    gs->players[0].hand_count = 2;
    gs->players[0].hand[0].type = CARD_SHA;
    gs->players[0].hand[1].type = CARD_SHAN;

    gs->players[1].hero = HERO_ZHAO_YUN;
    gs->players[1].personality = PERSON_RADICAL;
    gs->players[1].hp = 2;
    gs->players[1].hand_count = 2;
    gs->players[1].hand[0].type = CARD_SHA;
    gs->players[1].hand[1].type = CARD_WU_ZHONG;
}

static void test_ai_radical_prefers_shoot() {
    TEST_START("test_ai_radical_prefers_shoot");
    GameState gs;
    init_state(&gs);
    gs.players[1].personality = PERSON_RADICAL;

    Action action = ai_decide_action(&gs, 1);
    ASSERT_NE(action.action_type, 1, "激进派不应直接结束回合");
    TEST_PASS("test_ai_radical_prefers_shoot");
}

static void test_ai_conservative_holds_peach() {
    TEST_START("test_ai_conservative_holds_peach");
    GameState gs;
    init_state(&gs);
    gs.players[1].personality = PERSON_CONSERVATIVE;
    gs.players[1].hp = 3;

    Action action = ai_decide_action(&gs, 1);
    ASSERT_NE(action.card_index, 1, "保守派在高血量时不应优先使用桃");
    TEST_PASS("test_ai_conservative_holds_peach");
}

static void test_ai_psych_message_present() {
    TEST_START("test_ai_psych_message_present");
    GameState gs;
    init_state(&gs);
    gs.players[1].personality = PERSON_GAMBLER;

    ai_decide_action(&gs, 1);
    const char* msg = ai_get_last_psych_message();
    ASSERT_NOT_NULL(msg, "心理文字应返回非空字符串");
    TEST_PASS("test_ai_psych_message_present");
}

int main() {
    test_ai_radical_prefers_shoot();
    test_ai_conservative_holds_peach();
    test_ai_psych_message_present();
    printf("All AI tests finished.\n");
    return 0;
}
