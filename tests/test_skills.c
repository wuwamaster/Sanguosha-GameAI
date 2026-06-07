#include "../include/game_engine.h"
#include "test_utils.h"
#include <string.h>

extern int skill_can_use_sha(GameState* gs, int actor_idx);
extern int skill_can_convert(GameState* gs, int actor_idx, CardType type);
extern int skill_watch_stars(GameState* gs, int actor_idx, Card* out_top, int* out_count);
extern void skill_watch_stars_apply(GameState* gs, Card* new_order, int count);
extern int skill_empty_city_blocks_sha(GameState* gs, int target_idx);

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

static void test_zhuge_watch_stars_peek() {
    TEST_START("test_zhuge_watch_stars_peek");
    GameState gs;
    memset(&gs, 0, sizeof(gs));
    gs.player_count = 3;
    gs.players[0].hero = HERO_ZHU_GE_LIANG;
    gs.players[0].hp = 4;
    gs.players[1].hp = 4;
    gs.players[2].hp = 0;
    gs.pile_count = 5;
    gs.draw_pile[0] = (Card){CARD_SHA, 0};
    gs.draw_pile[1] = (Card){CARD_TAO, 0};
    gs.draw_pile[2] = (Card){CARD_SHAN, 0};
    gs.draw_pile[3] = (Card){CARD_GUO_CAI, 0};
    gs.draw_pile[4] = (Card){CARD_WU_ZHONG, 0};

    Card top[5];
    int count = 0;
    int result = skill_watch_stars(&gs, 0, top, &count);
    ASSERT_EQ(result, 2, "观星应返回存活角色数(2)");
    ASSERT_EQ(count, 2, "out_count应为2");
    ASSERT_EQ(top[0].type, CARD_WU_ZHONG, "第一张应为牌堆顶(无中)");
    ASSERT_EQ(top[1].type, CARD_GUO_CAI, "第二张应为过拆");
    TEST_PASS("test_zhuge_watch_stars_peek");
}

static void test_zhuge_watch_stars_apply() {
    TEST_START("test_zhuge_watch_stars_apply");
    GameState gs;
    memset(&gs, 0, sizeof(gs));
    gs.player_count = 3;
    gs.pile_count = 3;
    gs.draw_pile[0] = (Card){CARD_SHA, 0};
    gs.draw_pile[1] = (Card){CARD_GUO_CAI, 0};
    gs.draw_pile[2] = (Card){CARD_TAO, 0};

    Card order[3] = {
        {CARD_GUO_CAI, 0},
        {CARD_TAO, 0},
        {CARD_SHA, 0}
    };
    skill_watch_stars_apply(&gs, order, 3);
    ASSERT_EQ(gs.draw_pile[0].type, CARD_GUO_CAI, "观星后牌堆底应为过拆");
    ASSERT_EQ(gs.draw_pile[1].type, CARD_TAO, "观星后中间应为桃");
    ASSERT_EQ(gs.draw_pile[2].type, CARD_SHA, "观星后牌堆顶应为杀");
    TEST_PASS("test_zhuge_watch_stars_apply");
}

static void test_empty_city_blocks_sha() {
    TEST_START("test_empty_city_blocks_sha");
    GameState gs;
    memset(&gs, 0, sizeof(gs));
    gs.player_count = 2;
    gs.players[1].hero = HERO_ZHU_GE_LIANG;
    gs.players[1].hand_count = 0;
    gs.players[1].hp = 3;

    ASSERT_EQ(skill_empty_city_blocks_sha(&gs, 1), 1, "空城应阻挡杀");
    gs.players[1].hand_count = 1;
    ASSERT_EQ(skill_empty_city_blocks_sha(&gs, 1), 0, "有手牌不应阻挡杀");
    TEST_PASS("test_empty_city_blocks_sha");
}

static void test_empty_city_non_zhuge() {
    TEST_START("test_empty_city_non_zhuge");
    GameState gs;
    memset(&gs, 0, sizeof(gs));
    gs.player_count = 2;
    gs.players[1].hero = HERO_ZHANG_FEI;
    gs.players[1].hand_count = 0;

    ASSERT_EQ(skill_empty_city_blocks_sha(&gs, 1), 0, "非诸葛亮空城不应阻挡");
    TEST_PASS("test_empty_city_non_zhuge");
}

int main() {
    printf("\n=== Skills 测试 ===\n\n");
    printf("-- 咆哮 --\n");
    test_zhangfei_always_can_sha();
    test_non_zhangfei_skill_returns_zero();
    printf("\n-- 龙胆 --\n");
    test_zhaoyun_convert_both_directions();
    test_non_zhaoyun_cannot_convert();
    printf("\n-- 观星 --\n");
    test_zhuge_watch_stars_peek();
    test_zhuge_watch_stars_apply();
    printf("\n-- 空城 --\n");
    test_empty_city_blocks_sha();
    test_empty_city_non_zhuge();
    printf("\n=== 全部Skills测试完成 ===\n");
    return 0;
}
