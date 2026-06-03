#include "../include/game_engine.h"
#include "test_utils.h"

extern void game_init(GameState* gs, GameMode mode,
                      int player_is_lord,
                      HeroType player_hero,
                      HeroType ai_hero1, Personality ai_person1,
                      HeroType ai_hero2, Personality ai_person2);
extern ActionResult game_perform_action(GameState* gs, Action act);
extern int game_is_turn_over(GameState* gs);
extern void game_next_turn(GameState* gs);

static void test_game_init() {
    TEST_START("test_game_init");
    GameState gs;
    game_init(&gs, MODE_SINGLE, 0, HERO_ZHANG_FEI, HERO_ZHAO_YUN, PERSON_RADICAL, HERO_ZHU_GE_LIANG, PERSON_CONSERVATIVE);
    ASSERT_EQ(gs.player_count, 3, "单挑模式下应初始化三个角色");
    TEST_PASS("test_game_init");
}

static void test_turn_flow() {
    TEST_START("test_turn_flow");
    GameState gs;
    game_init(&gs, MODE_SINGLE, 0, HERO_ZHANG_FEI, HERO_ZHAO_YUN, PERSON_RADICAL, HERO_ZHU_GE_LIANG, PERSON_CONSERVATIVE);
    ASSERT_EQ(game_is_turn_over(&gs), 0, "新回合不应自动结束");
    TEST_PASS("test_turn_flow");
}

int main() {
    test_game_init();
    test_turn_flow();
    printf("Engine tests finished.\n");
    return 0;
}
