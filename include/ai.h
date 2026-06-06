#pragma once
#include "game_engine.h"

Action ai_decide_action(GameState* gs, int ai_idx);
int ai_should_use_tao_to_save(GameState* gs, int saver_idx, int dying_idx);
const char* ai_get_last_psych_message();
int ai_get_last_emotion();
void ai_generate_psych_message(GameState* gs, int ai_idx, Action act);