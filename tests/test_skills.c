#include "../include/game_engine.h"
#include "test_utils.h"
#include <string.h>

extern int skill_can_use_sha(GameState* gs, int actor_idx);
extern int skill_can_convert(GameState* gs, int actor_idx, CardType type);
extern int skill_watch_stars(GameState* gs, int actor_idx);

static void test_zhangfei_always_can_sha() {
    TEST_START("test_zhangfei_always_can_sha");
    GameState gs;
    memset(&gs, 0, sizeof(gs));
    gs.player_count = 2;
    gs.current_turn = 0;
    gs.players[0].hero = HERO_ZHANG_FEI;
    gs.players[0].hand_count = 2;
    gs.players[0].hand[0].type = CARD_SHA;
    gs.players[0].hand[1].type = CARD_SHA;

    ASSERT_EQ(skill_can_use_sha(&gs, 0), 1, "张飞应能使用多张杀");
    TEST_PASS("test_zhangfei_always_can_sha");
}

static void test_non_zhangfei_skill_returns_zero() {
    TEST_START("test_non_zhangfei_skill_returns_zero");
    GameState gs;
    memset(&gs, 0, sizeof(gs));
    gs.player_count = 2;
    gs.players[0].hero = HERO_ZHAO_YUN;

    ASSERT_EQ(skill_can_use_sha(&gs, 0), 0, "非张飞skill_can_use_sha应返回0");
    TEST_PASS("test_non_zhangfei_skill_returns_zero");
}

static void test_zhaoyun_convert_both_directions() {
    TEST_START("test_zhaoyun_convert_both_directions");
    GameState gs;
    memset(&gs, 0, sizeof(gs));
    gs.player_count = 2;
    gs.players[0].hero = HERO_ZHAO_YUN;

    ASSERT_EQ(skill_can_convert(&gs, 0, CARD_SHA), 1, "赵云应能杀转闪");
    ASSERT_EQ(skill_can_convert(&gs, 0, CARD_SHAN), 1, "赵云应能闪转杀");
    TEST_PASS("test_zhaoyun_convert_both_directions");
}

static void test_non_zhaoyun_cannot_convert() {
    TEST_START("test_non_zhaoyun_cannot_convert");
    GameState gs;
    memset(&gs, 0, sizeof(gs));
    gs.player_count = 2;
    gs.players[0].hero = HERO_ZHANG_FEI;

    ASSERT_EQ(skill_can_convert(&gs, 0, CARD_SHA), 0, "张飞不应能杀转闪");
    ASSERT_EQ(skill_can_convert(&gs, 0, CARD_SHAN), 0, "张飞不应能闪转杀");
    TEST_PASS("test_non_zhaoyun_cannot_convert");
}

static void test_zhuge_watch_stars() {
    TEST_START("test_zhuge_watch_stars");
    GameState gs;
    memset(&gs, 0, sizeof(gs));
    gs.player_count = 3;
    gs.players[0].hero = HERO_ZHU_GE_LIANG;

    ASSERT_EQ(skill_watch_stars(&gs, 0), 3, "诸葛亮观星应返回存活角色数");
    TEST_PASS("test_zhuge_watch_stars");
}

int main() {
    printf("\n=== Skills 测试 ===\n\n");
    test_zhangfei_always_can_sha();
    test_non_zhangfei_skill_returns_zero();
    test_zhaoyun_convert_both_directions();
    test_non_zhaoyun_cannot_convert();
    test_zhuge_watch_stars();
    printf("\n=== 全部Skills测试完成 ===\n");
    return 0;
}
