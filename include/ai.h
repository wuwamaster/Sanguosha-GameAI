#pragma once
#include "game_engine.h"

// 出牌决策
Action ai_decide_action(GameState* gs, int ai_idx);

// 濒死救人
int ai_should_use_tao_to_save(GameState* gs, int saver_idx, int dying_idx);

// 闪响应决策：返回 1=出闪，0=不出闪
// out_card_idx: 输出用的牌索引
// out_use_sha_as_shan: 输出是否用杀当闪（仅赵云）
int ai_decide_shan_response(GameState* gs, int defender_idx, 
                             int attacker_idx, int* out_card_idx,
                             int* out_use_sha_as_shan);

// 弃牌决策：返回要弃掉的手牌索引
int ai_decide_discard(GameState* gs, int ai_idx);

// 观星排序决策：直接修改 cards 数组的顺序
void ai_decide_star_order(GameState* gs, int ai_idx, 
                          Card* cards, int count);

// 心理文字
const char* ai_get_last_psych_message();
int ai_get_last_emotion();
void ai_generate_psych_message(GameState* gs, int ai_idx, Action act);
