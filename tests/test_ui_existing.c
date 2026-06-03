#include "ui.h"
#include <stdio.h>
#include <stdlib.h>

// 辅助函数：伪造一个 GameState
void fill_fake_gamestate(GameState* gs) {
    // 简单模式：玩家+1个AI
    gs->player_count = 2;
    gs->current_turn = 0;
    gs->game_over = 0;
    gs->winner = 0;

    // 玩家（张飞）
    gs->players[0].hero = HERO_ZHANG_FEI;
    gs->players[0].hp = 3;
    gs->players[0].max_hp = 4;
    gs->players[0].hand_count = 3;
    gs->players[0].hand[0].type = CARD_SHA;
    gs->players[0].hand[1].type = CARD_SHAN;
    gs->players[0].hand[2].type = CARD_TAO;

    // AI（赵云，激进派）
    gs->players[1].hero = HERO_ZHAO_YUN;
    gs->players[1].personality = PERSON_RADICAL;
    gs->players[1].hp = 2;
    gs->players[1].max_hp = 3;
    gs->players[1].hand_count = 2;
    gs->players[1].hand[0].type = CARD_SHA;
    gs->players[1].hand[1].type = CARD_GUO_CAI;
}

int main() {
    // 1. 测试 ui_init
    ui_init();

    // 2. 测试 ui_draw_game
    GameState gs;
    fill_fake_gamestate(&gs);
    ui_draw_game(&gs);

    // 3. 测试 ui_show_psych_message
    ui_show_psych_message("进攻是最好的防守！", 1);  // 愤怒表情
    ui_show_psych_message("稳妥一点，先留个闪。", 2); // 得意表情
    ui_show_psych_message("赌一把！无中生有！", 3);   // 奸笑表情

    // 4. 测试 ui_get_player_action（需要手动输入）
    printf("\n========== 现在测试玩家输入 ==========\n");
    Action act = ui_get_player_action(&gs);
    printf("你输入的行动: type=%d, card_index=%d, target=%d\n", act.action_type, act.card_index, act.target);

    // 5. 测试 ui_show_gameover
    ui_show_gameover(0);  // 玩家胜利
    ui_close();
    return 0;
}