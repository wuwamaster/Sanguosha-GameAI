#include <stdio.h>
#include "game_engine.h"
#include "ai.h"
#include "ui.h"
#include "file_io.h"

int main() {
    printf("=== 三国杀·人格分裂AI 项目骨架 ===\n");
    
    GameState gs;
    game_init(&gs, MODE_SINGLE,
              0,  // 玩家不是主公，仅单挑模式时此参数无实际影响
              HERO_ZHANG_FEI,
              HERO_ZHAO_YUN, PERSON_RADICAL,
              HERO_ZHU_GE_LIANG, PERSON_CONSERVATIVE);
    ui_init();
    
    // 模拟主循环
    while (!gs.game_over) {
        ui_draw_game(&gs);
        Action act;
        if (gs.current_turn == 0) {
            act = ui_get_player_action(&gs);
        } else {
            act = ai_decide_action(&gs, gs.current_turn - 1);
            ui_show_psych_message(ai_get_last_psych_message(), ai_get_last_emotion());
        }
        game_perform_action(&gs, act);
        if (game_is_turn_over(&gs)) {
            game_next_turn(&gs);
        }
    }
    
    ui_show_gameover(gs.winner);
    ui_close();
    return 0;
}