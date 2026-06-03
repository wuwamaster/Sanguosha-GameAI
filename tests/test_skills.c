#include "../include/game_engine.h"
#include "test_utils.h"
#include <string.h>

extern int skill_can_use_sha(GameState* gs, int actor_idx);
extern int skill_can_convert(GameState* gs, int actor_idx, CardType type);
extern int skill_watch_stars(GameState* gs, int actor_idx);

static void test_zhangfei_can_use_multiple_sha() {
    TEST_START("test_zhangfei_can_use_multiple_sha");
    GameState gs;
    memset(&gs, 0, sizeof(gs));
    gs.player_count = 2;
    gs.current_turn = 0;
    gs.players[0].hero = HERO_ZHANG_FEI;
    gs.players[0].hand_count = 2;
    gs.players[0].hand[0].type = CARD_SHA;
    gs.players[0].hand[1].type = CARD_SHA;

    int allowed = skill_can_use_sha(&gs, 0);
    ASSERT_EQ(allowed, 1, "张飞应允许在出牌阶段使用多张杀");

    TEST_PASS("test_zhangfei_can_use_multiple_sha");
}

static void test_zhaoyun_convert() {
    TEST_START("test_zhaoyun_convert");
    GameState gs;
    memset(&gs, 0, sizeof(gs));
    gs.player_count = 2;
    gs.players[0].hero = HERO_ZHAO_YUN;
    int can_convert = skill_can_convert(&gs, 0, CARD_SHA);
    ASSERT_EQ(can_convert, 1, "赵云应允许将杀转换为闪");
    can_convert = skill_can_convert(&gs, 0, CARD_SHAN);
    ASSERT_EQ(can_convert, 1, "赵云应允许将闪转换为杀");

    TEST_PASS("test_zhaoyun_convert");
}

static void test_zhuge_earth_watch() {
    TEST_START("test_zhuge_earth_watch");
    GameState gs;
    memset(&gs, 0, sizeof(gs));
    gs.player_count = 3;
    gs.current_turn = 0;
    gs.players[0].hero = HERO_ZHU_GE_LIANG;
    int count = skill_watch_stars(&gs, 0);
    ASSERT_EQ(count, 3, "诸葛亮观星应观察存活角色数张牌");

    TEST_PASS("test_zhuge_earth_watch");
}

int main() {
    test_zhangfei_can_use_multiple_sha();
    test_zhaoyun_convert();
    test_zhuge_earth_watch();
    printf("All skill tests finished.\n");
    return 0;
}
