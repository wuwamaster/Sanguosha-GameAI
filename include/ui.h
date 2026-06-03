#pragma once
#include "game_engine.h"

void ui_init();
void ui_draw_game(GameState* gs);
Action ui_get_player_action(GameState* gs);
void ui_show_psych_message(const char* message, int emotion_id);
void ui_show_gameover(int winner);
void ui_close();