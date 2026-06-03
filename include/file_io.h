#pragma once
#include "game_engine.h"

int load_card_deck(Card deck[], int max_cards);
int load_heroes(HeroType heroes[], int max_count);
int load_personalities(Personality personalities[], int max_count);
void save_game_result(const char* mode_str,
                      const char* player_hero,
                      const char* ai_hero,
                      const char* ai_personality,
                      int winner);
// 其他函数后续逐步添加