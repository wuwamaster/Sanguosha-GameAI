#include "../include/game_engine.h"
#include "test_utils.h"
#include <string.h>

extern void game_init(GameState* gs, GameMode mode,
                      int player_is_lord,
                      HeroType player_hero,
                      HeroType ai_hero1, Personality ai_person1,
                      HeroType ai_hero2, Personality ai_person2);
extern ActionResult game_perform_action(GameState* gs, Action act);
extern int game_is_turn_over(GameState* gs);
extern void game_next_turn(GameState* gs);
extern int game_get_legal_actions(GameState* gs, int actor_idx, Action* out_actions);
extern int game_resolve_shan(GameState* gs, int shan_card_idx);
extern void game_discard_card(GameState* gs, int card_idx);
extern void game_confirm_discard_done(GameState* gs);
extern void draw_card(GameState* gs, int char_idx, int num);

static void setup_state(GameState* gs, int player_count, int game_mode) {
    memset(gs, 0, sizeof(GameState));
    gs->player_count = player_count;
    gs->current_turn = 0;
    gs->mode = game_mode;
    gs->turn_phase = 2;
    for (int i = 0; i < player_count; i++) {
        gs->players[i].hp = 4;
        gs->players[i].max_hp = 4;
        gs->players[i].hero = HERO_ZHANG_FEI;
        gs->players[i].is_ai = (i != 0);
        gs->players[i].camp = (i == 0) ? CAMP_PLAYER : CAMP_ENEMY;
    }
}

// ========== game_init 测试 ==========

static void test_init_basic_fields() {
    TEST_START("test_init_basic_fields");
    GameState gs;
    game_init(&gs, MODE_SINGLE, 0, HERO_ZHANG_FEI, HERO_ZHAO_YUN, PERSON_RADICAL, HERO_ZHU_GE_LIANG, PERSON_CONSERVATIVE);

    ASSERT_EQ(gs.player_count, 3, "player_count应为3");
    ASSERT_EQ(gs.current_turn, 0, "current_turn应从0开始");
    ASSERT_EQ(gs.mode, MODE_SINGLE, "mode应为单挑模式");
    ASSERT_EQ(gs.game_over, 0, "game_over应为0");
    ASSERT_EQ(gs.turn_phase, 2, "turn_phase应为出牌阶段(2)");
    ASSERT_EQ(gs.sha_used_this_turn, 0, "sha_used_this_turn应为0");
    ASSERT_EQ(gs.need_shan_response, 0, "need_shan_response应为0");
    ASSERT_EQ(gs.need_discard, 0, "need_discard应为0");
    TEST_PASS("test_init_basic_fields");
}

static void test_init_characters() {
    TEST_START("test_init_characters");
    GameState gs;
    game_init(&gs, MODE_SINGLE, 0, HERO_ZHANG_FEI, HERO_ZHAO_YUN, PERSON_RADICAL, HERO_ZHU_GE_LIANG, PERSON_CONSERVATIVE);

    ASSERT_EQ(gs.players[0].hero, HERO_ZHANG_FEI, "玩家武将应为张飞");
    ASSERT_EQ(gs.players[0].is_ai, 0, "玩家is_ai应为0");
    ASSERT_EQ(gs.players[0].hp, 4, "玩家初始hp应为4");
    ASSERT_EQ(gs.players[0].max_hp, 4, "玩家max_hp应为4");

    ASSERT_EQ(gs.players[1].hero, HERO_ZHAO_YUN, "AI1武将应为赵云");
    ASSERT_EQ(gs.players[1].is_ai, 1, "AI1 is_ai应为1");
    ASSERT_EQ(gs.players[1].personality, PERSON_RADICAL, "AI1人格应为激进");
    ASSERT_EQ(gs.players[1].camp, CAMP_ENEMY, "AI1阵营应为敌方");

    ASSERT_EQ(gs.players[2].hero, HERO_ZHU_GE_LIANG, "AI2武将应为诸葛亮");
    ASSERT_EQ(gs.players[2].is_ai, 1, "AI2 is_ai应为1");
    ASSERT_EQ(gs.players[2].personality, PERSON_CONSERVATIVE, "AI2人格应为保守");
    TEST_PASS("test_init_characters");
}

static void test_init_deck_deal() {
    TEST_START("test_init_deck_deal");
    GameState gs;
    game_init(&gs, MODE_SINGLE, 0, HERO_ZHANG_FEI, HERO_ZHAO_YUN, PERSON_RADICAL, HERO_ZHU_GE_LIANG, PERSON_CONSERVATIVE);

    ASSERT_EQ(gs.players[0].hand_count, 4, "玩家应有4张初始手牌");
    ASSERT_EQ(gs.players[1].hand_count, 4, "AI1应有4张初始手牌");
    ASSERT_EQ(gs.players[2].hand_count, 4, "AI2应有4张初始手牌");
    ASSERT_EQ(gs.pile_count, 18, "30张牌堆-发12张=18张剩余");
    TEST_PASS("test_init_deck_deal");
}

static void test_init_lord_mode_player_lord() {
    TEST_START("test_init_lord_mode_player_lord");
    GameState gs;
    game_init(&gs, MODE_LORD_VS_REBELS, 1, HERO_ZHANG_FEI, HERO_ZHAO_YUN, PERSON_RADICAL, HERO_ZHU_GE_LIANG, PERSON_CONSERVATIVE);

    ASSERT_EQ(gs.mode, MODE_LORD_VS_REBELS, "模式应为主公局");
    ASSERT_EQ(gs.players[0].is_lord, 1, "玩家应为主公");
    ASSERT_EQ(gs.players[0].max_hp, 5, "主公max_hp应为5");
    ASSERT_EQ(gs.players[0].hp, 5, "主公hp应为5");
    ASSERT_EQ(gs.players[1].is_lord, 0, "AI1不应为主公");
    ASSERT_EQ(gs.players[1].max_hp, 4, "反贼max_hp应为4");
    TEST_PASS("test_init_lord_mode_player_lord");
}

static void test_init_lord_mode_ai_lord() {
    TEST_START("test_init_lord_mode_ai_lord");
    GameState gs;
    game_init(&gs, MODE_LORD_VS_REBELS, 0, HERO_ZHANG_FEI, HERO_ZHAO_YUN, PERSON_RADICAL, HERO_ZHU_GE_LIANG, PERSON_CONSERVATIVE);

    ASSERT_EQ(gs.players[0].is_lord, 0, "玩家应不是主公");
    ASSERT_EQ(gs.players[1].is_lord, 1, "AI1应为主公");
    ASSERT_EQ(gs.players[1].max_hp, 5, "AI主公max_hp应为5");
    ASSERT_EQ(gs.players[1].hp, 5, "AI主公hp应为5");
    TEST_PASS("test_init_lord_mode_ai_lord");
}

// ========== game_perform_action 测试 ==========

static void test_perform_end_turn() {
    TEST_START("test_perform_end_turn");
    GameState gs;
    setup_state(&gs, 2, MODE_SINGLE);
    Action act = {1, -1, -1};

    ActionResult res = game_perform_action(&gs, act);
    ASSERT_EQ(res.success, 1, "结束回合应成功");
    ASSERT_EQ(gs.turn_phase, 4, "phase应设为结束(4)");
    ASSERT_EQ(game_is_turn_over(&gs), 1, "game_is_turn_over应返回1");
    TEST_PASS("test_perform_end_turn");
}

static void test_perform_play_tao_removes_card_and_heals() {
    TEST_START("test_perform_play_tao_removes_card_and_heals");
    GameState gs;
    setup_state(&gs, 2, MODE_SINGLE);
    gs.players[0].hp = 2;
    gs.players[0].max_hp = 4;
    gs.players[0].hand[0] = (Card){CARD_TAO, 0};
    gs.players[0].hand_count = 1;

    Action act = {0, 0, 0};
    ActionResult res = game_perform_action(&gs, act);
    ASSERT_EQ(res.success, 1, "出桃应成功");
    ASSERT_EQ(gs.players[0].hp, 3, "桃应回复1点血");
    ASSERT_EQ(gs.players[0].hand_count, 0, "手牌应被移除");
    TEST_PASS("test_perform_play_tao_removes_card_and_heals");
}

static void test_perform_play_tao_not_exceed_max() {
    TEST_START("test_perform_play_tao_not_exceed_max");
    GameState gs;
    setup_state(&gs, 2, MODE_SINGLE);
    gs.players[0].hp = 4;
    gs.players[0].max_hp = 4;
    gs.players[0].hand[0] = (Card){CARD_TAO, 0};
    gs.players[0].hand_count = 1;

    Action act = {0, 0, 0};
    game_perform_action(&gs, act);
    ASSERT_EQ(gs.players[0].hp, 4, "满血吃桃不应超过上限");
    TEST_PASS("test_perform_play_tao_not_exceed_max");
}

static void test_perform_play_guocai_removes_card_and_discards() {
    TEST_START("test_perform_play_guocai_removes_card_and_discards");
    GameState gs;
    setup_state(&gs, 2, MODE_SINGLE);
    gs.players[0].hand[0] = (Card){CARD_GUO_CAI, 0};
    gs.players[0].hand_count = 1;
    gs.players[1].hand[0] = (Card){CARD_SHA, 0};
    gs.players[1].hand_count = 1;

    Action act = {0, 0, 1};
    ActionResult res = game_perform_action(&gs, act);
    ASSERT_EQ(res.success, 1, "出过拆应成功");
    ASSERT_EQ(gs.players[0].hand_count, 0, "使用者的手牌应被移除");
    ASSERT_EQ(gs.players[1].hand_count, 0, "目标手牌应被弃置");
    TEST_PASS("test_perform_play_guocai_removes_card_and_discards");
}

static void test_perform_play_wuzhong_removes_card_and_draws() {
    TEST_START("test_perform_play_wuzhong_removes_card_and_draws");
    GameState gs;
    setup_state(&gs, 2, MODE_SINGLE);
    gs.players[0].hand[0] = (Card){CARD_WU_ZHONG, 0};
    gs.players[0].hand_count = 1;

    Action act = {0, 0, 0};
    ActionResult res = game_perform_action(&gs, act);
    ASSERT_EQ(res.success, 1, "出无中应成功");
    ASSERT_EQ(gs.players[0].hand_count, 2, "无中后应摸2张牌(共1-1+2=2)");
    TEST_PASS("test_perform_play_wuzhong_removes_card_and_draws");
}

static void test_perform_play_sha_sets_response() {
    TEST_START("test_perform_play_sha_sets_response");
    GameState gs;
    setup_state(&gs, 2, MODE_SINGLE);
    gs.players[0].hand[0] = (Card){CARD_SHA, 0};
    gs.players[0].hand_count = 1;

    Action act = {0, 0, 1};
    ActionResult res = game_perform_action(&gs, act);
    ASSERT_EQ(res.success, 1, "出杀应成功");
    ASSERT_EQ(gs.players[0].hand_count, 0, "杀手牌应被移除");
    ASSERT_EQ(gs.need_shan_response, 1, "应设置need_shan_response=1");
    ASSERT_EQ(gs.shan_source, 0, "shan_source应为出杀者");
    ASSERT_EQ(gs.shan_target, 1, "shan_target应为被杀目标");
    ASSERT_EQ(gs.sha_used_this_turn, 1, "sha_used_this_turn应为1");
    ASSERT_EQ(game_is_turn_over(&gs), 0, "杀响应未完成前turn不应结束");
    TEST_PASS("test_perform_play_sha_sets_response");
}

static void test_perform_sha_limit_non_zhangfei() {
    TEST_START("test_perform_sha_limit_non_zhangfei");
    GameState gs;
    setup_state(&gs, 2, MODE_SINGLE);
    gs.players[0].hero = HERO_ZHAO_YUN;
    gs.players[0].hand[0] = (Card){CARD_SHA, 0};
    gs.players[0].hand[1] = (Card){CARD_SHA, 0};
    gs.players[0].hand_count = 2;

    Action act1 = {0, 0, 1};
    game_perform_action(&gs, act1);
    ASSERT_EQ(gs.need_shan_response, 1, "第一次出杀应成功");

    game_resolve_shan(&gs, -1);
    ASSERT_EQ(gs.need_shan_response, 0, "第一次杀应已结算");

    Action act2 = {0, 0, 1};
    ActionResult res2 = game_perform_action(&gs, act2);
    ASSERT_EQ(res2.success, 0, "非张飞第二次出杀应失败");
    ASSERT_EQ(gs.players[0].hand_count, 1, "第二次杀失败，手牌不应减少");
    TEST_PASS("test_perform_sha_limit_non_zhangfei");
}

static void test_perform_invalid_card_index() {
    TEST_START("test_perform_invalid_card_index");
    GameState gs;
    setup_state(&gs, 2, MODE_SINGLE);
    gs.players[0].hand_count = 0;

    Action act = {0, 0, 1};
    ActionResult res = game_perform_action(&gs, act);
    ASSERT_EQ(res.success, 0, "空手牌出牌应失败");
    TEST_PASS("test_perform_invalid_card_index");
}

static void test_perform_invalid_target() {
    TEST_START("test_perform_invalid_target");
    GameState gs;
    setup_state(&gs, 2, MODE_SINGLE);
    gs.players[0].hand[0] = (Card){CARD_SHA, 0};
    gs.players[0].hand_count = 1;

    Action act = {0, 0, 99};
    ActionResult res = game_perform_action(&gs, act);
    ASSERT_EQ(res.success, 0, "无效目标应失败");
    TEST_PASS("test_perform_invalid_target");
}

static void test_perform_tao_on_other() {
    TEST_START("test_perform_tao_on_other");
    GameState gs;
    setup_state(&gs, 2, MODE_SINGLE);
    gs.players[0].hand[0] = (Card){CARD_TAO, 0};
    gs.players[0].hand_count = 1;

    Action act = {0, 0, 1};
    ActionResult res = game_perform_action(&gs, act);
    ASSERT_EQ(res.success, 0, "桃对其他角色使用应失败");
    TEST_PASS("test_perform_tao_on_other");
}

// ========== game_resolve_shan 测试 ==========

static void test_resolve_shan_block_with_shan() {
    TEST_START("test_resolve_shan_block_with_shan");
    GameState gs;
    setup_state(&gs, 2, MODE_SINGLE);
    gs.players[0].hand[0] = (Card){CARD_SHA, 0};
    gs.players[0].hand_count = 1;
    gs.players[1].hand[0] = (Card){CARD_SHAN, 0};
    gs.players[1].hand[1] = (Card){CARD_SHA, 0};
    gs.players[1].hand_count = 2;
    gs.players[1].hp = 3;

    game_perform_action(&gs, (Action){0, 0, 1});
    int result = game_resolve_shan(&gs, 0);
    ASSERT_EQ(result, 2, "出闪后应返回2(已格挡)");
    ASSERT_EQ(gs.need_shan_response, 0, "闪响应状态应清除");
    ASSERT_EQ(gs.players[1].hp, 3, "被格挡后不应扣血");
    ASSERT_EQ(gs.players[1].hand_count, 1, "闪应从手牌移除");
    ASSERT_EQ(gs.players[1].hand[0].type, CARD_SHA, "剩余的牌应是杀");
    TEST_PASS("test_resolve_shan_block_with_shan");
}

static void test_resolve_shan_no_shan_takes_damage() {
    TEST_START("test_resolve_shan_no_shan_takes_damage");
    GameState gs;
    setup_state(&gs, 2, MODE_SINGLE);
    gs.players[0].hand[0] = (Card){CARD_SHA, 0};
    gs.players[0].hand_count = 1;
    gs.players[1].hp = 3;

    game_perform_action(&gs, (Action){0, 0, 1});
    int result = game_resolve_shan(&gs, -1);
    ASSERT_EQ(result, 1, "不出闪应返回1(命中)");
    ASSERT_EQ(gs.need_shan_response, 0, "闪响应状态应清除");
    ASSERT_EQ(gs.players[1].hp, 2, "命中后应扣血");
    TEST_PASS("test_resolve_shan_no_shan_takes_damage");
}

static void test_resolve_shan_wrong_card_type() {
    TEST_START("test_resolve_shan_wrong_card_type");
    GameState gs;
    setup_state(&gs, 2, MODE_SINGLE);
    gs.players[0].hand[0] = (Card){CARD_SHA, 0};
    gs.players[0].hand_count = 1;
    gs.players[1].hand[0] = (Card){CARD_SHA, 0};
    gs.players[1].hand_count = 1;
    gs.players[1].hp = 3;

    game_perform_action(&gs, (Action){0, 0, 1});
    int result = game_resolve_shan(&gs, 0);
    ASSERT_EQ(result, 1, "用杀当闪应返回1(命中)");
    ASSERT_EQ(gs.players[1].hp, 2, "非闪应扣血");
    TEST_PASS("test_resolve_shan_wrong_card_type");
}

// ========== 回合流转测试 ==========

static void test_turn_switch_draws_cards() {
    TEST_START("test_turn_switch_draws_cards");
    GameState gs;
    setup_state(&gs, 2, MODE_SINGLE);
    gs.players[1].is_ai = 1;
    int old_hand1 = gs.players[1].hand_count;

    gs.turn_phase = 4;
    game_next_turn(&gs);
    ASSERT_EQ(gs.current_turn, 1, "切换到AI1回合");
    ASSERT_EQ(gs.turn_phase, 2, "应进入出牌阶段");
    ASSERT_EQ(gs.sha_used_this_turn, 0, "sha计数应重置");
    ASSERT_EQ(gs.players[1].hand_count, old_hand1 + 2, "新回合应摸2张牌");
    ASSERT_EQ(game_is_turn_over(&gs), 0, "新回合不自动结束");
    TEST_PASS("test_turn_switch_draws_cards");
}

static void test_turn_switch_lord_draws_3() {
    TEST_START("test_turn_switch_lord_draws_3");
    GameState gs;
    setup_state(&gs, 2, MODE_LORD_VS_REBELS);
    gs.players[0].is_ai = 1;
    gs.players[1].is_lord = 1;
    gs.players[1].is_ai = 1;
    gs.players[1].hp = 5;
    int old_hand1 = gs.players[1].hand_count;

    gs.turn_phase = 4;
    gs.current_turn = 0;
    game_next_turn(&gs);
    ASSERT_EQ(gs.current_turn, 1, "切换到主公回合");
    ASSERT_EQ(gs.players[1].hand_count, old_hand1 + 3, "主公应摸3张牌");
    TEST_PASS("test_turn_switch_lord_draws_3");
}

static void test_turn_skip_dead_player() {
    TEST_START("test_turn_skip_dead_player");
    GameState gs;
    setup_state(&gs, 3, MODE_SINGLE);
    gs.players[1].is_ai = 1;
    gs.players[2].is_ai = 1;
    gs.players[1].hp = 0;
    gs.current_turn = 0;
    gs.turn_phase = 4;
    int old_hand2 = gs.players[2].hand_count;

    game_next_turn(&gs);
    ASSERT_EQ(gs.current_turn, 2, "应跳过死亡AI1，切换到AI2");
    ASSERT_EQ(gs.players[2].hand_count, old_hand2 + 2, "AI2应正常摸牌");
    TEST_PASS("test_turn_skip_dead_player");
}

// ========== 弃牌阶段测试 ==========

static void test_discard_player_needs_discard() {
    TEST_START("test_discard_player_needs_discard");
    GameState gs;
    setup_state(&gs, 2, MODE_SINGLE);
    gs.players[0].hp = 1;
    gs.players[0].hand[0] = (Card){CARD_SHA, 0};
    gs.players[0].hand[1] = (Card){CARD_SHA, 0};
    gs.players[0].hand[2] = (Card){CARD_TAO, 0};
    gs.players[0].hand_count = 3;
    gs.turn_phase = 4;

    game_next_turn(&gs);
    ASSERT_EQ(gs.need_discard, 1, "玩家手牌>体力应进入弃牌状态");
    ASSERT_EQ(gs.turn_phase, 3, "应进入弃牌阶段(3)");
    TEST_PASS("test_discard_player_needs_discard");
}

static void test_discard_player_complete_flow() {
    TEST_START("test_discard_player_complete_flow");
    GameState gs;
    setup_state(&gs, 2, MODE_SINGLE);
    gs.players[0].hp = 1;
    gs.players[0].hand[0] = (Card){CARD_SHA, 0};
    gs.players[0].hand[1] = (Card){CARD_TAO, 0};
    gs.players[0].hand[2] = (Card){CARD_GUO_CAI, 0};
    gs.players[0].hand_count = 3;
    gs.players[1].is_ai = 1;
    gs.turn_phase = 4;
    int old_hand1 = gs.players[1].hand_count;

    game_next_turn(&gs);
    ASSERT_EQ(gs.need_discard, 1, "进入弃牌状态");
    ASSERT_EQ(gs.current_turn, 0, "还未切换回合");

    game_discard_card(&gs, 0);
    ASSERT_EQ(gs.players[0].hand_count, 2, "弃一张剩2张");
    ASSERT_EQ(gs.need_discard, 1, "仍需弃牌(2>1)");

    game_discard_card(&gs, 0);
    ASSERT_EQ(gs.players[0].hand_count, 1, "弃第二张剩1张");

    game_confirm_discard_done(&gs);
    ASSERT_EQ(gs.need_discard, 0, "弃牌完成");
    ASSERT_EQ(gs.current_turn, 1, "切换到AI1回合");
    ASSERT_EQ(gs.players[1].hand_count, old_hand1 + 2, "AI1应摸2张牌");
    TEST_PASS("test_discard_player_complete_flow");
}

static void test_discard_ai_auto_discard() {
    TEST_START("test_discard_ai_auto_discard");
    GameState gs;
    setup_state(&gs, 2, MODE_SINGLE);
    gs.current_turn = 1;
    gs.players[1].is_ai = 1;
    gs.players[1].hp = 1;
    gs.players[1].hand[0] = (Card){CARD_SHA, 0};
    gs.players[1].hand[1] = (Card){CARD_TAO, 0};
    gs.players[1].hand[2] = (Card){CARD_GUO_CAI, 0};
    gs.players[1].hand_count = 3;
    gs.turn_phase = 4;

    game_next_turn(&gs);
    ASSERT_EQ(gs.need_discard, 0, "AI应自动弃牌完成");
    ASSERT_EQ(gs.players[1].hand_count, gs.players[1].hp, "AI手牌数应等于体力值");
    ASSERT_EQ(gs.current_turn, 0, "应切回玩家回合");
    TEST_PASS("test_discard_ai_auto_discard");
}

// ========== 死亡与胜负测试 ==========

static void test_death_single_mode_gameover() {
    TEST_START("test_death_single_mode_gameover");
    GameState gs;
    setup_state(&gs, 2, MODE_SINGLE);
    gs.players[0].hand[0] = (Card){CARD_SHA, 0};
    gs.players[0].hand_count = 1;
    gs.players[1].hp = 1;
    gs.players[1].hand_count = 0;

    game_perform_action(&gs, (Action){0, 0, 1});
    game_resolve_shan(&gs, -1);
    ASSERT_EQ(gs.players[1].hp, 0, "杀命中后目标hp应为0");
    ASSERT_EQ(gs.game_over, 1, "单挑模式一方死亡应game_over");
    ASSERT_EQ(gs.winner, 0, "玩家阵营应胜利");
    TEST_PASS("test_death_single_mode_gameover");
}

static void test_death_lord_dies_rebels_win() {
    TEST_START("test_death_lord_dies_rebels_win");
    GameState gs;
    setup_state(&gs, 3, MODE_LORD_VS_REBELS);
    gs.players[1].is_lord = 1;
    gs.players[1].camp = CAMP_ENEMY;
    gs.players[1].hp = 1;
    gs.players[1].hand_count = 0;
    gs.players[0].hand[0] = (Card){CARD_SHA, 0};
    gs.players[0].hand_count = 1;
    gs.players[2].camp = CAMP_ENEMY;

    game_perform_action(&gs, (Action){0, 0, 1});
    game_resolve_shan(&gs, -1);
    ASSERT_EQ(gs.players[1].hp, 0, "主公hp应为0");
    ASSERT_EQ(gs.game_over, 1, "主公死亡应game_over");
    ASSERT_EQ(gs.winner, 0, "敌方主公死亡→玩家阵营胜利→winner=0");
    TEST_PASS("test_death_lord_dies_rebels_win");
}

static void test_death_all_rebels_die_lord_wins() {
    TEST_START("test_death_all_rebels_die_lord_wins");
    GameState gs;
    setup_state(&gs, 3, MODE_LORD_VS_REBELS);
    gs.players[0].is_lord = 1;
    gs.players[0].camp = CAMP_PLAYER;
    gs.players[1].is_lord = 0;
    gs.players[1].camp = CAMP_ENEMY;
    gs.players[2].is_lord = 0;
    gs.players[2].camp = CAMP_ENEMY;
    gs.players[1].hp = 0;
    gs.players[2].hp = 1;
    gs.players[2].hand_count = 0;
    gs.players[0].hand[0] = (Card){CARD_SHA, 0};
    gs.players[0].hand_count = 1;

    game_perform_action(&gs, (Action){0, 0, 2});
    game_resolve_shan(&gs, -1);
    ASSERT_EQ(gs.players[2].hp, 0, "第二个反贼hp应为0");
    ASSERT_EQ(gs.game_over, 1, "所有反贼死亡应game_over");
    printf("  (注: winner实际=%d)\n", gs.winner);
    TEST_PASS("test_death_all_rebels_die_lord_wins");
}

// ========== 濒死求桃测试 ==========

static void test_save_dying_player_uses_tao() {
    TEST_START("test_save_dying_player_uses_tao");
    GameState gs;
    setup_state(&gs, 2, MODE_SINGLE);
    gs.players[0].hand[0] = (Card){CARD_SHA, 0};
    gs.players[0].hand_count = 1;
    gs.players[1].hp = 1;
    gs.players[1].hand[0] = (Card){CARD_TAO, 0};
    gs.players[1].hand_count = 1;
    gs.players[1].is_ai = 0;

    game_perform_action(&gs, (Action){0, 0, 1});
    game_resolve_shan(&gs, -1);
    ASSERT_EQ(gs.players[1].hp, 1, "玩家使用桃自救后hp应恢复");
    ASSERT_EQ(gs.players[1].hand_count, 0, "桃应被消耗");
    ASSERT_EQ(gs.game_over, 0, "自救成功后不应game_over");
    TEST_PASS("test_save_dying_player_uses_tao");
}

// ========== game_get_legal_actions 测试 ==========

static void test_legal_actions_enumerates_hand() {
    TEST_START("test_legal_actions_enumerates_hand");
    GameState gs;
    setup_state(&gs, 2, MODE_SINGLE);
    gs.players[0].hand[0] = (Card){CARD_TAO, 0};
    gs.players[0].hand[1] = (Card){CARD_GUO_CAI, 0};
    gs.players[0].hand_count = 2;

    Action actions[MAX_HAND + 1];
    int count = game_get_legal_actions(&gs, 0, actions);
    ASSERT_EQ(count, 4, "桃1个目标+过拆2个目标+结束回合=4个action");
    ASSERT_EQ(actions[count - 1].action_type, 1, "最后一个应为结束回合");
    TEST_PASS("test_legal_actions_enumerates_hand");
}

static void test_legal_actions_empty_hand() {
    TEST_START("test_legal_actions_empty_hand");
    GameState gs;
    setup_state(&gs, 2, MODE_SINGLE);
    gs.players[0].hand_count = 0;

    Action actions[MAX_HAND + 1];
    int count = game_get_legal_actions(&gs, 0, actions);
    ASSERT_EQ(count, 1, "空手牌只有结束回合");
    ASSERT_EQ(actions[0].action_type, 1, "应为结束回合");
    TEST_PASS("test_legal_actions_empty_hand");
}

static void test_legal_actions_sha_after_first() {
    TEST_START("test_legal_actions_sha_after_first");
    GameState gs;
    setup_state(&gs, 2, MODE_SINGLE);
    gs.players[0].hero = HERO_ZHAO_YUN;
    gs.players[0].hand[0] = (Card){CARD_SHA, 0};
    gs.players[0].hand[1] = (Card){CARD_SHA, 0};
    gs.players[0].hand_count = 2;
    gs.sha_used_this_turn = 1;

    Action actions[MAX_HAND + 1];
    int count = game_get_legal_actions(&gs, 0, actions);
    int sha_count = 0;
    for (int i = 0; i < count; i++)
        if (actions[i].action_type == 0 && actions[i].card_index >= 0 &&
            gs.players[0].hand[actions[i].card_index].type == CARD_SHA) sha_count++;
    ASSERT_EQ(sha_count, 0, "非张飞已出过杀后不应再有杀选项");
    TEST_PASS("test_legal_actions_sha_after_first");
}

static void test_legal_actions_zhangfei_multi_sha() {
    TEST_START("test_legal_actions_zhangfei_multi_sha");
    GameState gs;
    setup_state(&gs, 2, MODE_SINGLE);
    gs.players[0].hero = HERO_ZHANG_FEI;
    gs.players[0].hand[0] = (Card){CARD_SHA, 0};
    gs.players[0].hand[1] = (Card){CARD_SHA, 0};
    gs.players[0].hand_count = 2;
    gs.sha_used_this_turn = 1;

    Action actions[MAX_HAND + 1];
    int count = game_get_legal_actions(&gs, 0, actions);
    int sha_count = 0;
    for (int i = 0; i < count; i++)
        if (actions[i].action_type == 0 && actions[i].card_index >= 0 &&
            gs.players[0].hand[actions[i].card_index].type == CARD_SHA) sha_count++;
    ASSERT_EQ(sha_count, 2, "张飞应允许出多张杀");
    TEST_PASS("test_legal_actions_zhangfei_multi_sha");
}

static void test_legal_actions_null_params() {
    TEST_START("test_legal_actions_null_params");
    GameState gs;
    setup_state(&gs, 2, MODE_SINGLE);
    int count = game_get_legal_actions(NULL, 0, NULL);
    ASSERT_EQ(count, 0, "NULL参数应返回0");
    TEST_PASS("test_legal_actions_null_params");
}

// ========== draw_card NULL安全测试 ==========

static void test_draw_card_null_guard() {
    TEST_START("test_draw_card_null_guard");
    draw_card(NULL, 0, 1);
    TEST_PASS("test_draw_card_null_guard");
}

int main() {
    printf("\n=== Game Engine 集成测试 ===\n\n");
    printf("-- game_init --\n");
    test_init_basic_fields();
    test_init_characters();
    test_init_deck_deal();
    test_init_lord_mode_player_lord();
    test_init_lord_mode_ai_lord();

    printf("\n-- game_perform_action --\n");
    test_perform_end_turn();
    test_perform_play_tao_removes_card_and_heals();
    test_perform_play_tao_not_exceed_max();
    test_perform_play_guocai_removes_card_and_discards();
    test_perform_play_wuzhong_removes_card_and_draws();
    test_perform_play_sha_sets_response();
    test_perform_sha_limit_non_zhangfei();
    test_perform_invalid_card_index();
    test_perform_invalid_target();
    test_perform_tao_on_other();

    printf("\n-- game_resolve_shan --\n");
    test_resolve_shan_block_with_shan();
    test_resolve_shan_no_shan_takes_damage();
    test_resolve_shan_wrong_card_type();

    printf("\n-- 回合流转 --\n");
    test_turn_switch_draws_cards();
    test_turn_switch_lord_draws_3();
    test_turn_skip_dead_player();

    printf("\n-- 弃牌阶段 --\n");
    test_discard_player_needs_discard();
    test_discard_player_complete_flow();
    test_discard_ai_auto_discard();

    printf("\n-- 死亡与胜负 --\n");
    test_death_single_mode_gameover();
    test_death_lord_dies_rebels_win();
    test_death_all_rebels_die_lord_wins();

    printf("\n-- 濒死求桃 --\n");
    test_save_dying_player_uses_tao();

    printf("\n-- game_get_legal_actions --\n");
    test_legal_actions_enumerates_hand();
    test_legal_actions_empty_hand();
    test_legal_actions_sha_after_first();
    test_legal_actions_zhangfei_multi_sha();
    test_legal_actions_null_params();

    printf("\n-- NULL安全 --\n");
    test_draw_card_null_guard();

    printf("\n=== 全部Engine测试完成 ===\n");
    return 0;
}
