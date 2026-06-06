#pragma once
#include "game_engine.h"

/* 主菜单返回的玩家选择 */
typedef struct {
    GameMode    mode;             /* MODE_SINGLE / MODE_LORD_VS_REBELS */
    int         player_is_lord;   /* 仅主公局有效：1=玩家是主公，0=玩家是反贼 */
    HeroType    player_hero;
    HeroType    ai_hero[2];       /* AI 武将（随机分配） */
    Personality ai_person[2];     /* AI 人格（随机分配） */
    int         ai_count;         /* 单挑=1，主公局=2 */
} MenuChoice;

void ui_init();
void ui_draw_game(GameState* gs);
Action ui_get_player_action(GameState* gs);
void ui_show_psych_message(const char* message, int emotion_id);
void ui_show_gameover(int winner);
int ui_get_shan_response(GameState* gs);
int ui_get_discard_choice(GameState* gs, int remaining);
void ui_close();

/* 让 UI 持续刷新 seconds 秒（用于 AI 行动节奏控制、心理气泡可见时间）。
 * raylib 版会保持窗口响应；console 版可空实现或简单 sleep。 */
void ui_pause(GameState* gs, double seconds);

/* 通知 UI："actor 用 card 对 target_idx 出牌"。UI 把它作为最近出牌写入展示槽，
 * 主循环在 game_perform_action 之前调用。 */
void ui_show_card_played(int actor_idx, Card card, int target_idx);

/* 主菜单：玩家选择模式、武将、阵营。返回 1 确认，0 表示用户取消/关窗。
 * 调用前可先 ui_init()，也可不 init（函数会自行 ui_init）。 */
int ui_show_menu(MenuChoice* out);