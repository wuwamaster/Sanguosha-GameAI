#include "ai.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ========== 静态存储（供旧接口兼容）==========
static char g_last_psych_message[128] = "";
static int g_last_emotion = EMOTION_CALM;
static PsychResult g_last_result = {NULL, "", EMOTION_CALM, 0};

// ========== 辅助函数 ==========
static double random_double(double min, double max) {
    if (max <= min) return min;
    return (double)rand() / ((double)RAND_MAX + 1.0) * (max - min) + min;
}

// 根据牌类型获取显示概率
static double get_psych_show_rate(CardType card_type, int is_pass) {
    if (is_pass) {
        return PSYCH_SHOW_RATE_SKIP;
    }
    switch (card_type) {
        case CARD_SHA:    return PSYCH_SHOW_RATE_SHA;
        case CARD_TAO:    return PSYCH_SHOW_RATE_TAO;
        case CARD_GUO_CAI:
        case CARD_WU_ZHONG: return PSYCH_SHOW_RATE_GUO_WU;
        default:         return 0.3;
    }
}

// 获取情绪（根据人格）
static Emotion get_emotion_by_personality(Personality p) {
    switch (p) {
        case PERSON_RADICAL:    return EMOTION_ANGRY;
        case PERSON_CONSERVATIVE: return EMOTION_ANXIOUS;
        case PERSON_GAMBLER:    return EMOTION_EXCITED;
        default:                return EMOTION_CALM;
    }
}

// ========== 消息模板结构 ==========
typedef struct {
    CardType card_type;      // 牌类型，CARD_SHA等
    int is_pass;            // 是否是跳过行动
    const char* msg_dying;  // 濒死/敌人濒死场景
    const char* msg_no_card; // 敌人没牌场景
    const char* msg_many_cards; // 敌人多牌场景
    const char* msg_default; // 默认消息
} PsychTemplate;

// ========== 消息模板库 ==========
// 激进派模板
static const PsychTemplate TEMPLATES_RADICAL[CARD_WU_ZHONG + 1] = {
    [CARD_SHA]    = {CARD_SHA, 0,
                     "你只剩1血了，这一刀你躲不开！",
                     "你手里没闪，我杀定了！",
                     "牌多又怎样，照样砍你！",
                     "出杀！进攻是最好的防守！"},
    [CARD_TAO]    = {CARD_TAO, 0,
                     "血量见底，赶紧补一口！",
                     "",  // 桃不涉及敌人
                     "多补一口，以防万一。",
                     "先回点血再说。"},
    [CARD_GUO_CAI]= {CARD_GUO_CAI, 0,
                     "拆你最后的底牌！",
                     "",
                     "先拆你几张牌，别想反击！",
                     "拆你一张！"},
    [CARD_WU_ZHONG]= {CARD_WU_ZHONG, 0, "", "", "", "摸两张，准备大干一场！"},
};

// 保守派模板
static const PsychTemplate TEMPLATES_CONSERVATIVE[CARD_WU_ZHONG + 1] = {
    [CARD_SHA]    = {CARD_SHA, 0,
                     "先试探一下，看他有没有闪。",
                     "趁他没有防备，先试探一下。",
                     "这人血太厚，还是留闪防别人吧。",
                     "先试探一下，看他有没有闪。"},
    [CARD_TAO]    = {CARD_TAO, 0,
                     "小心为上，先补血。",
                     "",
                     "稳住血线，别浪。",
                     "先回点血。"},
    [CARD_GUO_CAI]= {CARD_GUO_CAI, 0,
                     "拆掉一个潜在威胁。",
                     "",
                     "拆掉一个潜在威胁。",
                     "拆掉一个潜在威胁。"},
    [CARD_WU_ZHONG]= {CARD_WU_ZHONG, 0, "", "", "", "先补牌，再观察局势。"},
};

// 赌徒派模板
static const PsychTemplate TEMPLATES_GAMBLER[CARD_WU_ZHONG + 1] = {
    [CARD_SHA]    = {CARD_SHA, 0,
                     "赌你救不回来！去死吧！",
                     "赌你手里没闪！干！",
                     "牌多又怎样，照样砍你！",
                     "赌你没有闪，干！"},
    [CARD_TAO]    = {CARD_TAO, 0,
                     "运气来了，先补个桃。",
                     "",
                     "赌一把，说不定能摸到好牌！",
                     "运气不错，补一口。"},
    [CARD_GUO_CAI]= {CARD_GUO_CAI, 0,
                     "看看你运气如何！",
                     "",
                     "看看你运气如何！",
                     "看看你运气如何！"},
    [CARD_WU_ZHONG]= {CARD_WU_ZHONG, 0, "", "", "", "摸两张，赌个好牌！"},
};

// 跳过消息（各人格独立）
static const char* SKIP_MESSAGES[] = {
    [PERSON_RADICAL]    = "暂时撤退，下一回合再发起猛攻。",
    [PERSON_CONSERVATIVE] = "稳住阵脚，别急着出手。",
    [PERSON_GAMBLER]    = "这把先不急，赌一把再说。",
};

// ========== 核心消息生成 ==========
static const char* select_message_by_scene(const PsychTemplate* t, 
                                           int target_hp, 
                                           int target_hand_count) {
    if (t == NULL) return NULL;

    // 濒死/敌人濒死
    if (target_hp <= 1 && t->msg_dying[0] != '\0') {
        return t->msg_dying;
    }
    // 敌人没手牌
    if (target_hand_count == 0 && t->msg_no_card[0] != '\0') {
        return t->msg_no_card;
    }
    // 敌人多手牌
    if (target_hand_count >= 3 && t->msg_many_cards[0] != '\0') {
        return t->msg_many_cards;
    }
    // 默认
    return t->msg_default;
}

static const PsychTemplate* get_template(Personality p, CardType card_type) {
    switch (p) {
        case PERSON_RADICAL:
            if (card_type >= 0 && card_type <= CARD_WU_ZHONG) {
                return &TEMPLATES_RADICAL[card_type];
            }
            return NULL;
        case PERSON_CONSERVATIVE:
            if (card_type >= 0 && card_type <= CARD_WU_ZHONG) {
                return &TEMPLATES_CONSERVATIVE[card_type];
            }
            return NULL;
        case PERSON_GAMBLER:
            if (card_type >= 0 && card_type <= CARD_WU_ZHONG) {
                return &TEMPLATES_GAMBLER[card_type];
            }
            return NULL;
        default:
            return NULL;
    }
}

// ========== 主接口实现 ==========
PsychResult ai_generate_psych_message_ex(GameState* gs, int ai_idx, Action act) {
    PsychResult result = {NULL, "", EMOTION_CALM, 0};

    if (gs == NULL || ai_idx < 0 || ai_idx >= gs->player_count) {
        return result;
    }

    Character* ai = &gs->players[ai_idx];
    if (!ai->is_ai) {
        return result;
    }

    Personality p = ai->personality;
    result.emotion = get_emotion_by_personality(p);

    // 判断是否跳过出牌
    int is_pass = (act.action_type == 1);

    // 获取牌类型
    CardType card_type = CARD_SHA; // 默认值
    if (!is_pass && act.card_index >= 0 && act.card_index < ai->hand_count) {
        card_type = ai->hand[act.card_index].type;
    }

    // ========== 概率决定是否显示心理活动 ==========
    double show_rate = get_psych_show_rate(card_type, is_pass);
    if (random_double(0.0, 1.0) > show_rate) {
        // 不显示心理活动
        result.thinking_msg = NULL;
        result.actual_msg = "";
        result.delay_ms = 0;
        g_last_result = result;
        return result;
    }

    // ========== 选择消息内容 ==========
    const char* msg = NULL;
    if (is_pass) {
        msg = SKIP_MESSAGES[p];
    } else {
        const PsychTemplate* t = get_template(p, card_type);
        if (t == NULL) {
            msg = "...";
        } else {
            // 获取目标信息
            int target_hp = 0;
            int target_hand_count = 0;
            if (act.target >= 0 && act.target < gs->player_count) {
                Character* target = &gs->players[act.target];
                target_hp = target->hp;
                target_hand_count = target->hand_count;
            }
            // 特殊处理：桃对自己使用，目标就是自己
            if (card_type == CARD_TAO) {
                target_hp = ai->hp;
                target_hand_count = ai->hand_count;
            }
            msg = select_message_by_scene(t, target_hp, target_hand_count);
            if (msg == NULL || msg[0] == '\0') {
                msg = t->msg_default;
            }
        }
    }

    if (msg == NULL) msg = "...";

    // ========== 决定是否"思考中" ==========
    result.thinking_msg = NULL;
    result.actual_msg = msg;
    result.delay_ms = 0;

    if (random_double(0.0, 1.0) < THINKING_RATE) {
        result.thinking_msg = "思考中🤔……";
        result.delay_ms = (int)(THINKING_MIN_MS + 
            random_double(0.0, THINKING_MAX_MS - THINKING_MIN_MS));
    }

    // 存储
    g_last_result = result;
    strncpy(g_last_psych_message, msg, sizeof(g_last_psych_message) - 1);
    g_last_psych_message[sizeof(g_last_psych_message) - 1] = '\0';
    g_last_emotion = result.emotion;

    return result;
}

// ========== 旧接口兼容实现 ==========
const char* ai_get_last_psych_message() {
    return g_last_psych_message;
}

int ai_get_last_emotion() {
    return g_last_emotion;
}

void ai_generate_psych_message(GameState* gs, int ai_idx, Action act) {
    PsychResult r = ai_generate_psych_message_ex(gs, ai_idx, act);
    (void)r; // 结果已存储到静态变量
}

// 获取最近一次完整结果（供UI层调用）
PsychResult ai_get_last_psych_result(void) {
    return g_last_result;
}
