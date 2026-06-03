#include "../include/ui.h"
#include "../include/game_engine.h"
#include "test_utils.h"
#include <string.h>

static void init_state(GameState* gs) {
    memset(gs, 0, sizeof(GameState));
    gs->player_count = 2;
    gs->current_turn = 0;
    gs->players[0].hero = HERO_ZHANG_FEI;
    gs->players[0].hp = 3;
    gs->players[0].max_hp = 4;
    gs->players[0].hand_count = 3;
    gs->players[0].hand[0].type = CARD_SHA;
    gs->players[0].hand[1].type = CARD_SHAN;
    gs->players[0].hand[2].type = CARD_TAO;
    gs->players[1].hero = HERO_ZHAO_YUN;
    gs->players[1].personality = PERSON_RADICAL;
    gs->players[1].hp = 2;
    gs->players[1].max_hp = 3;
    gs->players[1].hand_count = 2;
    gs->players[1].hand[0].type = CARD_SHA;
    gs->players[1].hand[1].type = CARD_GUO_CAI;
}

static void test_ui_draw() {
    TEST_START("test_ui_draw");
    GameState gs;
    init_state(&gs);
    ui_draw_game(&gs);
    TEST_PASS("test_ui_draw");
}

static void test_ui_psych() {
    TEST_START("test_ui_psych");
    ui_show_psych_message("测试心理文字", 2);
    TEST_PASS("test_ui_psych");
}

int main() {
    ui_init();
    test_ui_draw();
    test_ui_psych();
    printf("UI tests finished.\n");
    return 0;
}
