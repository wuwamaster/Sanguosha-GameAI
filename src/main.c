/*
 * main.c - 三国杀·人格分裂AI 主程序
 * 
 * 设计原则：
 * - AI 与玩家地位对等，每个决策点都有 AI 函数和 UI 函数两个入口
 * - main.c 只负责调度，不含游戏逻辑
 */

#include <stdio.h>
#include "game_engine.h"
#include "ai.h"
#include "ui.h"

/* ========================================================================
 * 主程序
 * ======================================================================== */

int main() {
    printf("=== 三国杀·人格分裂AI ===\n");
    fflush(stdout);

    /* 初始化 UI 和游戏 */
    ui_init();

    MenuChoice mc;
    printf("[DEBUG] Calling ui_show_menu...\n");
    fflush(stdout);
    if (!ui_show_menu(&mc)) {
        printf("[DEBUG] ui_show_menu returned 0, exiting\n");
        fflush(stdout);
        ui_close();
        return 0;
    }
    printf("[DEBUG] ui_show_menu succeeded, mode=%d, hero=%d, is_lord=%d\n", 
           mc.mode, mc.player_hero, mc.player_is_lord);
    fflush(stdout);

    GameState gs;
    printf("[DEBUG] Calling game_init...\n");
    fflush(stdout);
    game_init(&gs, mc.mode,
              mc.player_is_lord,
              mc.player_hero,
              mc.ai_hero[0], mc.ai_person[0],
              mc.ai_hero[1], mc.ai_person[1]);
    printf("[DEBUG] game_init done, player_count=%d, current_turn=%d, game_over=%d\n", 
           gs.player_count, gs.current_turn, gs.game_over);
    fflush(stdout);
    
    /* 发初始手牌并显示 */
    printf("[DEBUG] Calling game_deal_initial_cards...\n");
    fflush(stdout);
    game_deal_initial_cards(&gs);
    printf("[DEBUG] Initial hands dealt\n");
    fflush(stdout);
    
    printf("[DEBUG] Calling ui_show_initial_hand...\n");
    fflush(stdout);
    ui_show_initial_hand(&gs);
    printf("[DEBUG] Initial hand shown\n");
    fflush(stdout);

    /* ====================================================================
     * 游戏主循环
     * 
     * 每个需要决策的节点都遵循统一模式：
     *   if (是AI) → 调用 AI 函数
     *   else       → 调用 UI 函数
     * ==================================================================== */
    
    while (!gs.game_over) {
        printf("[DEBUG] turn=%d phase=%d game_over=%d need_shan=%d need_discard=%d\n", 
               gs.current_turn, gs.turn_phase, gs.game_over, 
               gs.need_shan_response, gs.need_discard);
        fflush(stdout);
        
        /* -----------------------------------------------------------
         * 1. 绘制游戏画面（每帧都绘制）
         * ----------------------------------------------------------- */
        ui_draw_game(&gs);
        
        /* -----------------------------------------------------------
         * 显示游戏事件（闪响应、濒死救人、过河拆桥等）
         * ----------------------------------------------------------- */
        ui_show_events(&gs);

        /* -----------------------------------------------------------
         * 2. 闪响应决策（回合外）
         *    - 玩家：ui_get_shan_response()
         *    - AI：ai_decide_shan_response()
         * ----------------------------------------------------------- */
        if (gs.need_shan_response) {
            int card_idx = -1;
            int use_shan = 0;
            int use_sha_as_shan = 0;

            if (gs.players[gs.shan_target].is_ai) {
                /* AI 决策：是否出闪，是否用杀当闪（赵云） */
                ui_pause(&gs, 0.5);  // AI响应前停顿
                use_shan = ai_decide_shan_response(&gs, gs.shan_target, 
                                                   gs.shan_source, 
                                                   &card_idx, &use_sha_as_shan);
                ui_pause(&gs, 0.5);  // AI响应后停顿
            } else {
                /* 玩家决策：UI 获取选择 */
                card_idx = ui_get_shan_response(&gs);
                use_shan = (card_idx >= 0);
                /* 玩家赵云：暂时不支持用杀当闪的选择，后续可扩展 */
            }

            game_resolve_shan(&gs, use_shan ? card_idx : -1, use_sha_as_shan);
            continue;  // 处理完后重新检查状态
        }

        /* -----------------------------------------------------------
         * 3. 弃牌决策
         *    - 玩家：ui_get_discard_choice()
         *    - AI：ai_decide_discard()
         * ----------------------------------------------------------- */
        if (gs.need_discard) {
            int card_idx = -1;

            if (gs.players[gs.current_turn].is_ai) {
                /* AI 决策：选择弃哪张 */
                ui_pause(&gs, 0.6);  // AI弃牌前停顿
                card_idx = ai_decide_discard(&gs, gs.current_turn);
                ui_pause(&gs, 0.5);  // AI弃牌后停顿
            } else {
                /* 玩家决策：UI 获取选择 */
                int excess = gs.players[0].hand_count - gs.players[0].hp;
                card_idx = ui_get_discard_choice(&gs, excess);
            }

            if (card_idx >= 0) {
                game_discard_card(&gs, card_idx);
            }

            /* 检查弃牌是否完成 */
            if (gs.players[gs.current_turn].hand_count <= 
                gs.players[gs.current_turn].hp) {
                game_confirm_discard_done(&gs);
            }
            continue;
        }

        /* -----------------------------------------------------------
         * 4. 观星决策（仅诸葛亮）
         *    - 玩家：ui_get_star_choice()
         *    - AI：ai_decide_star_order()
         * ----------------------------------------------------------- */
        if (gs.need_star_choice) {
            if (gs.players[gs.current_turn].is_ai) {
                /* AI 决策：观星排序 */
                ui_pause(&gs, 0.8);  // AI观星前停顿
                ai_decide_star_order(&gs, gs.current_turn,
                                     gs.star_watch_cards, gs.star_watch_count);
                ui_pause(&gs, 0.5);  // AI观星后停顿
            } else {
                /* 玩家决策：UI 获取选择 */
                ui_get_star_choice(&gs);
            }
            game_confirm_star_choice(&gs);
            continue;
        }

        /* -----------------------------------------------------------
         * 5. 回合阶段推进（准备阶段、摸牌阶段）
         *    这两个阶段通常不需要决策，直接推进
         * ----------------------------------------------------------- */
        if (gs.turn_phase < 2) {
            ui_pause(&gs, 0.5);
            game_advance_phase(&gs);
            continue;
        }

        /* -----------------------------------------------------------
         * 6. 出牌阶段决策
         *    - 玩家：ui_get_player_action()
         *    - AI：ai_decide_action()
         * ----------------------------------------------------------- */
        Action act;

        if (gs.players[gs.current_turn].is_ai) {
            /* AI 决策 */
            ui_pause(&gs, 0.8);
            act = ai_decide_action(&gs, gs.current_turn);
            
            /* 生成并显示 AI 心理文案 */
            ai_generate_psych_message(&gs, gs.current_turn, act);
            ui_show_psych_message(
                ai_get_last_psych_message(),
                ai_get_last_emotion()
            );
            ui_pause(&gs, 1.5);
        } else {
            /* 玩家决策 */
            act = ui_get_player_action(&gs);
        }

        /* -----------------------------------------------------------
         * 7. 显示出牌信息
         * ----------------------------------------------------------- */
        if (act.action_type == 0 && 
            act.card_index >= 0 && 
            act.card_index < gs.players[gs.current_turn].hand_count) {
            Card played = gs.players[gs.current_turn].hand[act.card_index];
            ui_show_card_played(gs.current_turn, played, act.target);
        }

        /* -----------------------------------------------------------
         * 8. 执行动作
         * ----------------------------------------------------------- */
        game_perform_action(&gs, act);

        /* AI 出牌后短暂停顿，让玩家看清 */
        if (act.action_type == 0 && gs.current_turn != 0) {
            ui_pause(&gs, 0.4);
        }

        /* -----------------------------------------------------------
         * 9. 检查回合是否结束
         * ----------------------------------------------------------- */
        if (game_is_turn_over(&gs)) {
            game_next_turn(&gs);
        }
    }

    /* 游戏结束 */
    ui_show_gameover(gs.winner);
    ui_close();
    return 0;
}
