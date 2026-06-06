#include <stdio.h>
#include "game_engine.h"
#include "ai.h"
#include "ui.h"
#include "file_io.h"

int main() {
    printf("=== 三国杀·人格分裂AI ===\n");

    ui_init();

    MenuChoice mc;
    if (!ui_show_menu(&mc)) {
        ui_close();
        return 0;
    }

    GameState gs;
    game_init(&gs, mc.mode,
              mc.player_is_lord,
              mc.player_hero,
              mc.ai_hero[0], mc.ai_person[0],
              mc.ai_hero[1], mc.ai_person[1]);

    while (!gs.game_over) {
        ui_draw_game(&gs);

        if (gs.need_shan_response) {
            int shan_idx = -1;
            if (gs.shan_target == 0) {
                shan_idx = ui_get_shan_response(&gs);
            } else {
                Character* ai = &gs.players[gs.shan_target];
                for (int i = 0; i < ai->hand_count; i++) {
                    if (ai->hand[i].type == CARD_SHAN) {
                        shan_idx = i;
                        break;
                    }
                }
            }
            game_resolve_shan(&gs, shan_idx);
            continue;
        }

        if (gs.need_discard) {
            int excess = gs.players[0].hand_count - gs.players[0].hp;
            int idx = ui_get_discard_choice(&gs, excess);
            if (idx >= 0) {
                game_discard_card(&gs, idx);
            }
            if (gs.players[0].hand_count <= gs.players[0].hp) {
                game_confirm_discard_done(&gs);
            }
            continue;
        }

        Action act;
        if (gs.current_turn == 0) {
            act = ui_get_player_action(&gs);
        } else {
            /* AI 思考节奏：先停一下，让玩家看清局势 */
            ui_pause(&gs, 0.6);
            act = ai_decide_action(&gs, gs.current_turn);
            ui_show_psych_message(ai_get_last_psych_message(), ai_get_last_emotion());
            /* 心理气泡可见时间 */
            ui_pause(&gs, 1.0);
        }

        /* 通知出牌区（仅出牌动作） */
        if (act.action_type == 0
            && act.card_index >= 0
            && act.card_index < gs.players[gs.current_turn].hand_count) {
            Card played = gs.players[gs.current_turn].hand[act.card_index];
            ui_show_card_played(gs.current_turn, played, act.target);
        }

        game_perform_action(&gs, act);

        /* 出牌后短暂停顿，让出牌区/血量变化看得见 */
        if (act.action_type == 0 && gs.current_turn != 0) {
            ui_pause(&gs, 0.4);
        }

        if (game_is_turn_over(&gs)) {
            game_next_turn(&gs);
        }
    }

    ui_show_gameover(gs.winner);
    ui_close();
    return 0;
}
