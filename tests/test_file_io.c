#include "../include/game_engine.h"
#include "../include/file_io.h"
#include "test_utils.h"
#include <string.h>
#include <stdio.h>

// ========== 牌堆加载测试 ==========

static void test_load_card_deck_count() {
    TEST_START("test_load_card_deck_count");
    Card deck[64];
    int count = load_card_deck(deck, 64);
    ASSERT_NE(count, 0, "cards.txt 应被读取返回牌数");
    ASSERT_EQ(count, 30, "牌堆应为30张");
    printf("  加载了 %d 张牌\n", count);
    TEST_PASS("test_load_card_deck_count");
}

static void test_load_card_deck_content_valid() {
    TEST_START("test_load_card_deck_content_valid");
    Card deck[64];
    int count = load_card_deck(deck, 64);

    int sha_cnt = 0, shan_cnt = 0, tao_cnt = 0, guo_cnt = 0, wu_cnt = 0;
    for (int i = 0; i < count; i++) {
        switch (deck[i].type) {
            case CARD_SHA: sha_cnt++; break;
            case CARD_SHAN: shan_cnt++; break;
            case CARD_TAO: tao_cnt++; break;
            case CARD_GUO_CAI: guo_cnt++; break;
            case CARD_WU_ZHONG: wu_cnt++; break;
        }
    }
    printf("  杀:%d 闪:%d 桃:%d 过拆:%d 无中:%d\n",
           sha_cnt, shan_cnt, tao_cnt, guo_cnt, wu_cnt);
    ASSERT_EQ(sha_cnt, 12, "杀应为12张");
    ASSERT_EQ(shan_cnt, 8, "闪应为8张");
    ASSERT_EQ(tao_cnt, 5, "桃应为5张");
    ASSERT_EQ(guo_cnt, 3, "过河拆桥应为3张");
    ASSERT_EQ(wu_cnt, 2, "无中生有应为2张");
    TEST_PASS("test_load_card_deck_content_valid");
}

static void test_load_card_deck_null() {
    TEST_START("test_load_card_deck_null");
    int count = load_card_deck(NULL, 64);
    ASSERT_EQ(count, 0, "NULL参数应返回0");
    TEST_PASS("test_load_card_deck_null");
}

// ========== 武将加载测试 ==========

static void test_load_heroes_count() {
    TEST_START("test_load_heroes_count");
    HeroType heroes[10];
    int count = load_heroes(heroes, 10);
    ASSERT_EQ(count, 3, "默认应有3个武将");
    ASSERT_EQ(heroes[0], HERO_ZHANG_FEI, "第一个应为张飞");
    ASSERT_EQ(heroes[1], HERO_ZHAO_YUN, "第二个应为赵云");
    ASSERT_EQ(heroes[2], HERO_ZHU_GE_LIANG, "第三个应为诸葛亮");
    printf("  加载了 %d 个武将\n", count);
    TEST_PASS("test_load_heroes_count");
}

// ========== 人格加载测试 ==========

static void test_load_personalities_count() {
    TEST_START("test_load_personalities_count");
    PersonalityWeights weights[10];
    int count = load_personalities(weights, 10);
    ASSERT_EQ(count, 3, "应有3种人格");

    // 验证激进派
    ASSERT_TRUE(weights[0].sha_weight > 1.0, "激进派杀权重大于1");
    ASSERT_TRUE(weights[0].aggressiveness > 0.5, "激进派激进程度高");

    // 验证保守派
    ASSERT_EQ(weights[1].personality, PERSON_CONSERVATIVE, "第二个人格应为保守派");
    ASSERT_TRUE(weights[1].shan_weight > 1.0, "保守派闪权重大于1");

    // 验证赌徒派
    ASSERT_EQ(weights[2].personality, PERSON_GAMBLER, "第三个人格应为赌徒派");
    printf("  加载了 %d 种人格\n", count);
    for (int i = 0; i < count; i++) {
        printf("  %s: sha=%.2f shan=%.2f tao=%.2f guo=%.2f wu=%.2f agg=%.2f hp=%d\n",
               weights[i].name, weights[i].sha_weight, weights[i].shan_weight,
               weights[i].tao_weight, weights[i].guo_weight, weights[i].wu_weight,
               weights[i].aggressiveness, weights[i].hp_threshold);
    }
    TEST_PASS("test_load_personalities_count");
}

// ========== 保存/统计测试 ==========

static void test_save_and_stats() {
    TEST_START("test_save_and_stats");

    // 清除旧记录
    remove("game_results.txt");

    // 写入几条测试记录
    save_game_result("SINGLE", "ZHANG_FEI", "ZHAO_YUN", "RADICAL", 0);
    save_game_result("SINGLE", "ZHAO_YUN", "ZHU_GE_LIANG", "CONSERVATIVE", 1);
    save_game_result("SINGLE", "ZHU_GE_LIANG", "ZHANG_FEI", "GAMBLER", 0);
    save_game_result("LORD_VS_REBELS", "ZHANG_FEI", "ZHAO_YUN", "RADICAL", 0);

    // 读取统计
    Stats s = get_stats();
    ASSERT_EQ(s.total_games, 4, "应有4局记录");
    ASSERT_EQ(s.player_wins, 3, "玩家应胜3局");
    ASSERT_EQ(s.ai_wins, 1, "AI应胜1局");

    printf("  总局数: %d, 玩家胜: %d, AI胜: %d\n", s.total_games, s.player_wins, s.ai_wins);
    stats_print(&s);

    TEST_PASS("test_save_and_stats");
}

static void test_save_personalities_file() {
    TEST_START("test_save_personalities_file");

    PersonalityWeights pws[3];
    int count = load_personalities(pws, 3);
    ASSERT_EQ(count, 3, "应加载3种人格");

    // 修改后保存
    pws[0].aggressiveness = 0.99;
    int ok = save_personalities(pws, 3);
    ASSERT_EQ(ok, 1, "保存人格应成功");

    // 重新加载验证
    PersonalityWeights pws2[3];
    int count2 = load_personalities(pws2, 3);
    ASSERT_EQ(count2, 3, "重新加载应有3种人格");
    ASSERT_TRUE(pws2[0].aggressiveness > 0.9, "修改后的激进程度应被保存");

    // 恢复默认值
    pws[0].aggressiveness = 0.85;
    save_personalities(pws, 3);

    TEST_PASS("test_save_personalities_file");
}

// ========== 主函数 ==========

int main() {
    printf("\n=== FileIO 测试 ===\n\n");

    test_load_card_deck_count();
    test_load_card_deck_content_valid();
    test_load_card_deck_null();

    printf("\n--- 武将测试 ---\n");
    test_load_heroes_count();

    printf("\n--- 人格测试 ---\n");
    test_load_personalities_count();

    printf("\n--- 保存/统计测试 ---\n");
    test_save_and_stats();

    printf("\n--- 人格保存测试 ---\n");
    test_save_personalities_file();

    printf("\n=== 全部FileIO测试完成 ===\n");
    return 0;
}
