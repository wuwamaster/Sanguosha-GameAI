#pragma once
#include "game_engine.h"

// ========== 概率常量 ==========
#define PSYCH_SHOW_RATE_SHA      0.60   // 出杀显示心理活动概率
#define PSYCH_SHOW_RATE_TAO      0.50   // 出桃显示概率
#define PSYCH_SHOW_RATE_GUO_WU   0.40   // 过拆/无中显示概率
#define PSYCH_SHOW_RATE_PASS     0.30   // 结束回合显示概率
#define PSYCH_SHOW_RATE_SKIP     0.70   // 跳过出牌显示概率

#define THINKING_RATE            0.35   // 思考中模式触发概率
#define THINKING_MIN_MS          1500
#define THINKING_MAX_MS          3000

// ========== 情绪枚举 ==========
typedef enum {
    EMOTION_CALM = 0,       // 平静
    EMOTION_CONFUSED = 1,   // 疑惑
    EMOTION_ANGRY = 2,      // 激进/愤怒
    EMOTION_ANXIOUS = 3,    // 保守/紧张
    EMOTION_EXCITED = 4,    // 赌徒/兴奋
} Emotion;

// ========== 心理活动结果 ==========
typedef struct {
    const char* thinking_msg;   // 思考中消息（如"思考中🤔……"），NULL表示不显示
    const char* actual_msg;    // 实际心理活动消息
    Emotion emotion;            // 情绪类型
    int delay_ms;              // 延迟毫秒，0表示不延迟
} PsychResult;

// ========== 出牌决策 ==========
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

// ========== 心理活动系统 ==========
// 新接口：返回完整心理结果（含思考延迟信息）
PsychResult ai_generate_psych_message_ex(GameState* gs, int ai_idx, Action act);

// 旧接口兼容：返回最近一次的实际消息
const char* ai_get_last_psych_message();
int ai_get_last_emotion();

// 旧接口：生成心理活动（调用新接口，内部存储供旧getter使用）
void ai_generate_psych_message(GameState* gs, int ai_idx, Action act);

// 获取最近一次完整心理结果（供UI层调用，包含思考延迟信息）
PsychResult ai_get_last_psych_result(void);
