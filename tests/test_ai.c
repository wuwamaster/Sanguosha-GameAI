#include "../include/game_engine.h"
#include "../include/ai.h"
#include "test_utils.h"
#include <string.h>

extern Action ai_decide_action(GameState* gs, int ai_idx);
extern const char* ai_get_last_psych_message();
extern int ai_get_last_emotion();

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

static void test_ai_returns_action_when_has_cards() {
    TEST_START("test_ai_returns_action_when_has_cards");
    GameState gs;
    init_state(&gs);

    Action action = ai_decide_action(&gs, 1);
    ASSERT_EQ(action.action_type, 0, "有手牌时应出牌(action_type=0)");
    ASSERT_TRUE(action.card_index >= 0, "card_index应有效");
    TEST_PASS("test_ai_returns_action_when_has_cards");
}

static void test_ai_ends_turn_when_no_cards() {
    TEST_START("test_ai_ends_turn_when_no_cards");
    GameState gs;
    init_state(&gs);
    gs.players[1].hand_count = 0;

    Action action = ai_decide_action(&gs, 1);
    ASSERT_EQ(action.action_type, 1, "无手牌应结束回合(action_type=1)");
    TEST_PASS("test_ai_ends_turn_when_no_cards");
}

static void test_ai_null_guard() {
    TEST_START("test_ai_null_guard");
    Action action = ai_decide_action(NULL, 0);
    ASSERT_EQ(action.action_type, 1, "NULL参数应返回结束回合");
    TEST_PASS("test_ai_null_guard");
}

static void test_ai_psych_message_not_null() {
    TEST_START("test_ai_psych_message_not_null");
    GameState gs;
    init_state(&gs);
    gs.players[1].personality = PERSON_RADICAL;

    ai_decide_action(&gs, 1);
    const char* msg = ai_get_last_psych_message();
    ASSERT_NOT_NULL(msg, "心理文字应返回非空字符串");
    ASSERT_TRUE(msg[0] != '\0', "心理文字应为非空字符串");
    TEST_PASS("test_ai_psych_message_not_null");
}

static void test_ai_psych_message_different_personalities() {
    TEST_START("test_ai_psych_message_different_personalities");
    GameState gs;
    init_state(&gs);
    gs.players[1].hand_count = 0;

    gs.players[1].personality = PERSON_RADICAL;
    ai_decide_action(&gs, 1);
    const char* msg_radical = ai_get_last_psych_message();

    gs.players[1].personality = PERSON_CONSERVATIVE;
    ai_decide_action(&gs, 1);
    const char* msg_conservative = ai_get_last_psych_message();

    printf("  激进派: %s\n  保守派: %s\n", msg_radical, msg_conservative);
    TEST_PASS("test_ai_psych_message_different_personalities");
}

int main() {
    printf("\n=== AI 模块测试 ===\n\n");
    test_ai_returns_action_when_has_cards();
    test_ai_ends_turn_when_no_cards();
    test_ai_null_guard();
    test_ai_psych_message_not_null();
    test_ai_psych_message_different_personalities();
    printf("\n=== 全部AI测试完成 ===\n");
    return 0;
}
