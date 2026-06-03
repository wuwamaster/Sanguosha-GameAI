#include "../include/game_engine.h"
#include "../include/file_io.h"
#include "test_utils.h"
#include <string.h>

extern int load_card_deck(Card deck[], int max_cards);
extern void save_game_result(const char* mode_str,
                             const char* player_hero,
                             const char* ai_hero,
                             const char* ai_personality,
                             int winner);

static void test_load_card_deck() {
    TEST_START("test_load_card_deck");
    Card deck[64];
    int count = load_card_deck(deck, 64);
    ASSERT_NE(count, 0, "cards.txt 应该被正确读取且返回牌数");
    TEST_PASS("test_load_card_deck");
}

static void test_save_game_result() {
    TEST_START("test_save_game_result");
    save_game_result("单挑", "张飞", "赵云", "激进派", 0);
    TEST_PASS("test_save_game_result");
}

int main() {
    test_load_card_deck();
    test_save_game_result();
    printf("File IO tests finished.\n");
    return 0;
}
