#pragma once
#include "game_engine.h"

// ========== 统计数据定义 ==========
#define MAX_HERO_NAME 32
#define MAX_MODE_NAME 32

typedef struct {
    int total_games;           // 总局数
    int player_wins;           // 玩家胜场数
    int ai_wins;               // AI胜场数

    // 按武将统计
    int hero_wins[3];          // 对应HERO_ZHANG_FEI, HERO_ZHAO_YUN, HERO_ZHU_GE_LIANG
    int hero_games[3];

    // 按模式统计
    int mode_wins[2];          // 对应MODE_SINGLE, MODE_LORD_VS_REBELS
    int mode_games[2];

    // 按人格统计
    int personality_wins[3];   // 对应PERSON_RADICAL, PERSON_CONSERVATIVE, PERSON_GAMBLER
    int personality_games[3];
} Stats;

// ========== 数据加载 ==========

// 加载牌堆（从 data/cards.txt 读取，每行一张牌名）
int load_card_deck(Card deck[], int max_cards);

// 加载武将列表（从 data/heroes.txt 读取，每行一个武将名；文件不存在时返回默认3武将）
int load_heroes(HeroType heroes[], int max_count);

// 加载人格权重参数（从 data/personalities.txt 读取）
int load_personalities(PersonalityWeights weights[], int max_count);

// 加载人格枚举列表（简单返回3种人格类型）
int load_personality_types(Personality personalities[], int max_count);

// ========== 保存与统计 ==========

// 保存一局游戏结果到 game_results.txt
void save_game_result(const char* mode_str,
                      const char* player_hero,
                      const char* ai_hero,
                      const char* ai_personality,
                      int winner);

// 读取游戏记录，生成统计数据
Stats get_stats(void);

// 打印统计信息到控制台
void stats_print(const Stats* s);

// ========== 后台管理（保存配置）==========

// 保存武将配置到 heroes.txt
int save_heroes(const char* hero_names[], int count);

// 保存人格权重到 personalities.txt
int save_personalities(const PersonalityWeights weights[], int count);
