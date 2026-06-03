#include "../include/game_engine.h"
#include "test_utils.h"
#include <string.h>

static void init_state(GameState* gs) {
    memset(gs, 0, sizeof(GameState));
    gs->player_count = 2;
    gs->current_turn = 0;
    gs->game_over = 0;
    gs->winner = 0;

    gs->players[0].hero = HERO_ZHANG_FEI;
    gs->players[0].hp = 3;
    gs->players[0].max_hp = 4;
    gs->players[0].hand_count = 4;
    gs->players[0].hand[0].type = CARD_SHA;
    gs->players[0].hand[1].type = CARD_TAO;
    gs->players[0].hand[2].type = CARD_GUO_CAI;
    gs->players[0].hand[3].type = CARD_WU_ZHONG;

    gs->players[1].hero = HERO_ZHAO_YUN;
    gs->players[1].hp = 3;
    gs->players[1].max_hp = 4;
    gs->players[1].hand_count = 2;
    gs->players[1].hand[0].type = CARD_SHAN;
    gs->players[1].hand[1].type = CARD_SHA;
}

static void test_shan_damage() {
    TEST_START("test_shan_damage");
    GameState gs;
    init_state(&gs);

    Card sha = {CARD_SHA, 0};
    apply_card_effect(&gs, 0, sha, 1);
    ASSERT_EQ(gs.players[1].hp, 2, "杀命中后目标血量应减1");

    TEST_PASS("test_shan_damage");
}

static void test_tao_heal() {
    TEST_START("test_tao_heal");
    GameState gs;
    init_state(&gs);
    gs.players[0].hp = 1;

    Card tao = {CARD_TAO, 0};
    apply_card_effect(&gs, 0, tao, 0);
    ASSERT_EQ(gs.players[0].hp, 2, "桃使用后应恢复1点血");

    TEST_PASS("test_tao_heal");
}

static void test_guo_cai_discard() {
    TEST_START("test_guo_cai_discard");
    GameState gs;
    init_state(&gs);

    Card guo = {CARD_GUO_CAI, 0};
    apply_card_effect(&gs, 0, guo, 1);
    ASSERT_EQ(gs.players[1].hand_count, 1, "过河拆桥后目标手牌数应减少1");

    TEST_PASS("test_guo_cai_discard");
}

static void test_wu_zhong_draw() {
    TEST_START("test_wu_zhong_draw");
    GameState gs;
    init_state(&gs);

    Card wu = {CARD_WU_ZHONG, 0};
    apply_card_effect(&gs, 0, wu, 0);
    ASSERT_EQ(gs.players[0].hand_count, 6, "无中生有后手牌数应增加2");

    TEST_PASS("test_wu_zhong_draw");
}

int main() {
    test_shan_damage();
    test_tao_heal();
    test_guo_cai_discard();
    test_wu_zhong_draw();
    printf("All card tests finished.\n");
    return 0;
}
