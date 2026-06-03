#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ---------- 常量定义 ----------
#define MAX_HAND 10
#define MAX_PILE 50
#define MAX_PLAYERS 3   // 玩家 + 最多2个AI

// ---------- 枚举类型 ----------
typedef enum {
    CARD_SHA = 1,
    CARD_SHAN,
    CARD_TAO,
    CARD_GUO_CAI,
    CARD_WU_ZHONG
} CardType;

typedef enum {
    HERO_ZHANG_FEI,
    HERO_ZHAO_YUN,
    HERO_ZHU_GE_LIANG
} HeroType;

typedef enum {
    PERSON_RADICAL,
    PERSON_CONSERVATIVE,
    PERSON_GAMBLER
} Personality;

// ---------- 数据结构 ----------
typedef struct {
    CardType type;
    int id;
} Card;

typedef enum {
    MODE_SINGLE = 0,
    MODE_LORD_VS_REBELS = 1
} GameMode;

typedef enum {
    CAMP_PLAYER = 0,
    CAMP_ENEMY = 1
} Camp;

typedef struct {
    HeroType hero;
    Personality personality;  // 仅AI有效
    int hp;
    int max_hp;
    Card hand[MAX_HAND];
    int hand_count;
    int is_ai;      // 1=AI, 0=玩家
    int is_lord;    // 是否为主公
    Camp camp;      // 所属阵营
} Character;

typedef struct {
    Character players[MAX_PLAYERS];
    int player_count;      // 实际角色数（1玩家 + AI数量）
    int current_turn;      // 0~player_count-1
    Card draw_pile[MAX_PILE];
    int pile_count;
    Card discard_pile[MAX_PILE];
    int discard_count;
    GameMode mode;         // 游戏模式
    int player_is_lord;    // 仅在主公局模式有效
    int game_over;
    int winner;            // 0:玩家阵营胜利, 1:敌方阵营胜利
} GameState;

typedef struct {
    int action_type;       // 0:出牌, 1:结束回合
    int card_index;        // 手牌索引（出牌时使用）
    int target;            // 目标角色索引 (-1表示无目标)
} Action;

typedef struct {
    int success;
    int game_over;
    int winner;
} ActionResult;

// ---------- Engine 对外接口 ----------
void game_init(GameState* gs, GameMode mode,
               int player_is_lord,
               HeroType player_hero,
               HeroType ai_hero1, Personality ai_person1,
               HeroType ai_hero2, Personality ai_person2);
ActionResult game_perform_action(GameState* gs, Action act);
int game_is_turn_over(GameState* gs);
void game_next_turn(GameState* gs);
int game_get_legal_actions(GameState* gs, int actor_idx, Action* out_actions);

// ---------- 内部辅助函数（供其他Engine内文件调用，不对外）----------
void draw_card(GameState* gs, int char_idx, int num);
void apply_card_effect(GameState* gs, int user_idx, Card card, int target_idx);
int save_dying(GameState* gs, int dying_idx, int start_idx);