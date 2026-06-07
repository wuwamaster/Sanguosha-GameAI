#include "../include/game_engine.h"
#include "test_utils.h"
#include <string.h>

extern void apply_card_effect(GameState* gs, int user_idx, Card card, int target_idx);
extern void draw_card(GameState* gs, int char_idx, int num);
extern int save_dying(GameState* gs, int dying_idx, int start_idx);
extern const char* card_type_name(CardType type);
extern int card_is_offensive(CardType type);
extern int card_is_defensive(CardType type);

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

// ========== 原有基础测试 ==========

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

// ========== 新增边界测试 ==========

static void test_tao_not_exceed_max_hp() {
    TEST_START("test_tao_not_exceed_max_hp");
    GameState gs;
    init_state(&gs);
    gs.players[0].hp = 4;
    gs.players[0].max_hp = 4;

    Card tao = {CARD_TAO, 0};
    apply_card_effect(&gs, 0, tao, 0);
    ASSERT_EQ(gs.players[0].hp, 4, "满血吃桃不应超过上限");
    TEST_PASS("test_tao_not_exceed_max_hp");
}

static void test_tao_full_heal_capped() {
    TEST_START("test_tao_full_heal_capped");
    GameState gs;
    init_state(&gs);
    gs.players[0].hp = 3;
    gs.players[0].max_hp = 4;

    Card tao = {CARD_TAO, 0};
    apply_card_effect(&gs, 0, tao, 0);
    ASSERT_EQ(gs.players[0].hp, 4, "桃应回复但不超过上限");
    TEST_PASS("test_tao_full_heal_capped");
}

static void test_guocai_empty_hand() {
    TEST_START("test_guocai_empty_hand");
    GameState gs;
    init_state(&gs);
    gs.players[1].hand_count = 0;

    Card guo = {CARD_GUO_CAI, 0};
    apply_card_effect(&gs, 0, guo, 1);
    ASSERT_EQ(gs.players[1].hand_count, 0, "空手牌过拆应无影响");
    TEST_PASS("test_guocai_empty_hand");
}

static void test_sha_kill_triggers_death() {
    TEST_START("test_sha_kill_triggers_death");
    GameState gs;
    init_state(&gs);
    gs.players[1].hp = 1;
    gs.players[1].hand_count = 0;
    gs.players[0].hand_count = 1;
    gs.players[0].hand[0].type = CARD_SHA;
    gs.mode = MODE_SINGLE;

    Card sha = {CARD_SHA, 0};
    apply_card_effect(&gs, 0, sha, 1);
    ASSERT_EQ(gs.players[1].hp, 0, "杀致死后目标hp应为0");
    ASSERT_EQ(gs.game_over, 1, "单挑模式应game_over");
    TEST_PASS("test_sha_kill_triggers_death");
}

static void test_draw_card_from_empty_pile() {
    TEST_START("test_draw_card_from_empty_pile");
    GameState gs;
    init_state(&gs);
    gs.pile_count = 0;
    gs.discard_count = 0;
    gs.players[0].hand_count = 0;

    draw_card(&gs, 0, 3);
    ASSERT_EQ(gs.players[0].hand_count, 3, "空牌堆应使用fallback给3张牌");
    ASSERT_EQ(gs.players[0].hand[0].type, CARD_SHA, "fallback卡牌应为杀");
    TEST_PASS("test_draw_card_from_empty_pile");
}

static void test_draw_card_from_discard_recycle() {
    TEST_START("test_draw_card_from_discard_recycle");
    GameState gs;
    init_state(&gs);
    gs.pile_count = 0;
    gs.discard_pile[0] = (Card){CARD_TAO, 99};
    gs.discard_count = 1;
    gs.players[0].hand_count = 0;

    draw_card(&gs, 0, 1);
    ASSERT_EQ(gs.players[0].hand_count, 1, "应从弃牌堆回收摸牌");
    ASSERT_EQ(gs.players[0].hand[0].type, CARD_TAO, "摸到的应为回收的桃");
    ASSERT_EQ(gs.discard_count, 0, "弃牌堆应被清空");
    TEST_PASS("test_draw_card_from_discard_recycle");
}

static void test_save_dying_no_tao_dies() {
    TEST_START("test_save_dying_no_tao_dies");
    GameState gs;
    init_state(&gs);
    gs.mode = MODE_SINGLE;
    gs.players[1].hp = 0;
    gs.players[1].hand_count = 0;
    gs.players[0].hand_count = 1;
    gs.players[0].hand[0].type = CARD_SHA;

    int saved = save_dying(&gs, 1, gs.current_turn);
    ASSERT_EQ(saved, 0, "无桃应返回0(未被救)");
    ASSERT_EQ(gs.players[1].hp, 0, "无桃时hp仍为0");
    TEST_PASS("test_save_dying_no_tao_dies");
}

static void test_card_type_name_valid() {
    TEST_START("test_card_type_name_valid");
    ASSERT_EQ(strcmp(card_type_name(CARD_SHA), "杀"), 0, "杀名称");
    ASSERT_EQ(strcmp(card_type_name(CARD_SHAN), "闪"), 0, "闪名称");
    ASSERT_EQ(strcmp(card_type_name(CARD_TAO), "桃"), 0, "桃名称");
    ASSERT_EQ(strcmp(card_type_name(CARD_GUO_CAI), "过河拆桥"), 0, "过拆名称");
    ASSERT_EQ(strcmp(card_type_name(CARD_WU_ZHONG), "无中生有"), 0, "无中名称");
    TEST_PASS("test_card_type_name_valid");
}

static void test_card_is_offensive() {
    TEST_START("test_card_is_offensive");
    ASSERT_EQ(card_is_offensive(CARD_SHA), 1, "杀是进攻牌");
    ASSERT_EQ(card_is_offensive(CARD_GUO_CAI), 1, "过拆是进攻牌");
    ASSERT_EQ(card_is_offensive(CARD_TAO), 0, "桃不是进攻牌");
    ASSERT_EQ(card_is_offensive(CARD_SHAN), 0, "闪不是进攻牌");
    TEST_PASS("test_card_is_offensive");
}

static void test_card_is_defensive() {
    TEST_START("test_card_is_defensive");
    ASSERT_EQ(card_is_defensive(CARD_SHAN), 1, "闪是防御牌");
    ASSERT_EQ(card_is_defensive(CARD_TAO), 1, "桃是防御牌");
    ASSERT_EQ(card_is_defensive(CARD_SHA), 0, "杀不是防御牌");
    ASSERT_EQ(card_is_defensive(CARD_GUO_CAI), 0, "过拆不是防御牌");
    TEST_PASS("test_card_is_defensive");
}

int main() {
    printf("\n=== Cards 边界测试 ===\n\n");
    printf("-- 基础效果 --\n");
    test_shan_damage();
    test_tao_heal();
    test_guo_cai_discard();
    test_wu_zhong_draw();
    printf("\n-- 边界 --\n");
    test_tao_not_exceed_max_hp();
    test_tao_full_heal_capped();
    test_guocai_empty_hand();
    test_sha_kill_triggers_death();
    test_draw_card_from_empty_pile();
    test_draw_card_from_discard_recycle();
    test_save_dying_no_tao_dies();
    printf("\n-- 卡牌工具 --\n");
    test_card_type_name_valid();
    test_card_is_offensive();
    test_card_is_defensive();
    printf("\n=== 全部Cards测试完成 ===\n");
    return 0;
}
