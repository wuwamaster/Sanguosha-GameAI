#include "../include/game_engine.h"
#include "../include/file_io.h"
#include "test_utils.h"
#include <string.h>
#include <stdio.h>

extern int load_card_deck(Card deck[], int max_cards);
extern void save_game_result(const char* mode_str,
                             const char* player_hero,
                             const char* ai_hero,
                             const char* ai_personality,
                             int winner);

static void test_load_card_deck_count() {
    TEST_START("test_load_card_deck_count");
    Card deck[64];
    int count = load_card_deck(deck, 64);
    ASSERT_NE(count, 0, "cards.txt 应被读取返回牌数");
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
    ASSERT_TRUE(count > 0, "应加载到卡牌");
    TEST_PASS("test_load_card_deck_content_valid");
}

static void test_load_card_deck_null() {
    TEST_START("test_load_card_deck_null");
    int count = load_card_deck(NULL, 64);
    ASSERT_EQ(count, 0, "NULL参数应返回0");
    TEST_PASS("test_load_card_deck_null");
}

static void test_save_game_result_writes_file() {
    TEST_START("test_save_game_result_writes_file");
    const char* fname = "game_results.txt";
    remove(fname);

    save_game_result("单挑测试", "张飞", "赵云", "激进派", 0);

    FILE* f = fopen(fname, "r");
    if (f != NULL) {
        char buf[256];
        if (fgets(buf, sizeof(buf), f)) {
            printf("  写入内容: %s", buf);
        }
        fclose(f);
    }
    ASSERT_TRUE(f != NULL, "game_results.txt应被创建");
    TEST_PASS("test_save_game_result_writes_file");
}

int main() {
    printf("\n=== FileIO 测试 ===\n\n");
    test_load_card_deck_count();
    test_load_card_deck_content_valid();
    test_load_card_deck_null();
    test_save_game_result_writes_file();
    printf("\n=== 全部FileIO测试完成 ===\n");
    return 0;
}
