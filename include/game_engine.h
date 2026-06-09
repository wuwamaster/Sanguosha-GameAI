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

// ---------- 游戏事件系统 ----------
typedef enum {
    EVENT_GAME_START,       // 游戏开始
    EVENT_TURN_START,       // 回合开始
    EVENT_PHASE_CHANGE,     // 阶段变化
    EVENT_TURN_END,         // 回合结束
    EVENT_DRAW_CARDS,       // 摸牌
    EVENT_CARD_PLAYED,      // 出牌
    EVENT_SHAN_RESPONSE,    // 闪响应
    EVENT_TAO_SAVE,         // 濒死救人
    EVENT_DISMANTLE,        // 过河拆桥
    EVENT_DMG_TAKEN,        // 受到伤害
    EVENT_DEATH,            // 死亡
    EVENT_SKILL_USED        // 技能触发
} GameEventType;

typedef struct {
    GameEventType type;
    int actor;              // 行动者索引
    int target;             // 目标索引（-1表示无）
    CardType card_type;     // 涉及的牌类型（CARD_SHA等）
    char message[128];      // 显示消息
    double shown_at;        // 首次显示时间（用于控制显示时长）
} GameEvent;

#define MAX_EVENTS 10       // 最大事件日志数
#define EVENT_VISIBLE_SEC 3.0  // 事件显示时长（秒）


// AI人格权重参数（从personalities.txt读取）
typedef struct {
    Personality personality;
    double sha_weight;       // 杀权重
    double shan_weight;      // 闪权重
    double tao_weight;       // 桃权重
    double guo_weight;       // 过河拆桥权重
    double wu_weight;        // 无中生有权重
    double aggressiveness;   // 激进程度 (0.0~1.0)
    int hp_threshold;        // 使用桃的血量阈值
    char name[32];           // 人格名称
} PersonalityWeights;

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

    int turn_phase;        // 0:准备 1:摸牌 2:出牌 3:弃牌 4:结束
    int sha_used_this_turn;

    int need_shan_response;
    int shan_source;
    int shan_target;

    int need_discard;

    int need_star_choice;
    Card star_watch_cards[5];
    int star_watch_count;
    int star_current_slots[5];

    // ---------- 事件日志 ----------
    GameEvent events[MAX_EVENTS];
    int event_count;       // 当前事件数
    int event_head;        // 环形缓冲区头指针
} GameState;

typedef struct {
    int action_type;       // 0:出牌, 1:结束回合
    int card_index;        // 手牌索引（出牌时使用）
    int target;            // 目标角色索引 (-1表示无目标)
    int use_longdan_sha;   // 龙胆：出闪时，是否用闪当杀（仅赵云）
    int use_longdan_shan;  // 龙胆：被杀时，是否用杀当闪（仅赵云，0=不出杀挡，1=用杀挡）
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
void game_deal_initial_cards(GameState* gs);  // 发初始手牌
ActionResult game_perform_action(GameState* gs, Action act);
int game_is_turn_over(GameState* gs);
void game_next_turn(GameState* gs);
int game_get_legal_actions(GameState* gs, int actor_idx, Action* out_actions);
int game_resolve_shan(GameState* gs, int shan_card_idx, int use_sha_as_shan);
void game_discard_card(GameState* gs, int card_idx);
void game_confirm_discard_done(GameState* gs);

int game_advance_phase(GameState* gs);
void game_swap_star_slots(GameState* gs, int slot_a, int slot_b);
void game_confirm_star_choice(GameState* gs);

// ---------- 事件系统接口 ----------
void game_push_event(GameState* gs, GameEventType type, int actor, 
                     int target, CardType card_type, const char* message);
void game_clear_events(GameState* gs);
void game_clear_expired_events(GameState* gs, double now);
int game_get_event_count(GameState* gs);
GameEvent* game_get_event(GameState* gs, int index);

// ---------- 技能接口 ----------
int skill_can_use_sha(GameState* gs, int actor_idx);
int skill_can_convert(GameState* gs, int actor_idx, CardType type);
int skill_watch_stars(GameState* gs, int actor_idx, Card* out_top, int* out_count);
void skill_watch_stars_apply(GameState* gs, Card* new_order, int count);
int skill_empty_city_blocks_sha(GameState* gs, int target_idx);

// ---------- Cards 工具接口 ----------
const char* card_type_name(CardType type);
int card_is_offensive(CardType type);
int card_is_defensive(CardType type);

// ---------- 内部辅助函数（供其他Engine内文件调用，不对外）----------
void draw_card(GameState* gs, int char_idx, int num);
void apply_card_effect(GameState* gs, int user_idx, Card card, int target_idx);
int save_dying(GameState* gs, int dying_idx, int start_idx);
