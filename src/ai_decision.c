#include "ai.h"
#include <stdlib.h>
#include <stdio.h>

static PersonalityWeights get_personality_weights(Personality personality) {
    PersonalityWeights w;
    w.personality = personality;
    w.sha_weight = 1.0;
    w.shan_weight = 1.0;
    w.tao_weight = 1.0;
    w.guo_weight = 1.0;
    w.wu_weight = 1.0;
    w.aggressiveness = 0.5;
    w.hp_threshold = 2;
    strncpy(w.name, "默认", sizeof(w.name));
    w.name[sizeof(w.name) - 1] = '\0';

    switch (personality) {
        case PERSON_RADICAL:
            w.sha_weight = 1.4;
            w.shan_weight = 0.8;
            w.tao_weight = 0.8;
            w.guo_weight = 1.0;
            w.wu_weight = 1.1;
            w.aggressiveness = 0.9;
            w.hp_threshold = 2;
            strncpy(w.name, "激进派", sizeof(w.name));
            break;
        case PERSON_CONSERVATIVE:
            w.sha_weight = 0.8;
            w.shan_weight = 1.2;
            w.tao_weight = 1.3;
            w.guo_weight = 1.0;
            w.wu_weight = 0.9;
            w.aggressiveness = 0.3;
            w.hp_threshold = 3;
            strncpy(w.name, "保守派", sizeof(w.name));
            break;
        case PERSON_GAMBLER:
            w.sha_weight = 1.0;
            w.shan_weight = 0.9;
            w.tao_weight = 1.0;
            w.guo_weight = 1.2;
            w.wu_weight = 1.4;
            w.aggressiveness = 0.6;
            w.hp_threshold = 2;
            strncpy(w.name, "赌徒派", sizeof(w.name));
            break;
        default:
            break;
    }
    w.name[sizeof(w.name) - 1] = '\0';
    return w;
}

static double random_double(double min, double max) {
    if (max <= min) return min;
    return min + (double)rand() / ((double)RAND_MAX + 1.0) * (max - min);
}

static int is_enemy(GameState* gs, int actor_idx, int target_idx) {
    if (gs == NULL || actor_idx < 0 || actor_idx >= gs->player_count ||
        target_idx < 0 || target_idx >= gs->player_count) {
        return 0;
    }
    return gs->players[actor_idx].camp != gs->players[target_idx].camp;
}

static double score_action(GameState* gs, int ai_idx, Action act, const PersonalityWeights* w) {
    if (gs == NULL || w == NULL) return -1000.0;
    if (act.action_type == 1) return 0.0;

    Character* ai = &gs->players[ai_idx];
    if (act.card_index < 0 || act.card_index >= ai->hand_count) return -1000.0;
    Card card = ai->hand[act.card_index];
    Character* target = NULL;
    if (act.target >= 0 && act.target < gs->player_count) {
        target = &gs->players[act.target];
    }

    double score = 0.0;
    switch (card.type) {
        case CARD_SHA:
            score = w->sha_weight;
            if (target) {
                if (is_enemy(gs, ai_idx, act.target)) {
                    score += 0.6;
                } else {
                    // 对队友使用杀，给予很大的负收益
                    score -= 5.0;
                }
                score += (4 - target->hp) * 0.15;
                if (target->hand_count == 0) score += 0.2;
            }
            break;
        case CARD_TAO:
            if (ai->hp < ai->max_hp) {
                score = w->tao_weight + (ai->max_hp - ai->hp) * 0.3;
            } else {
                score = -5.0;
            }
            break;
        case CARD_GUO_CAI:
            score = w->guo_weight;
            if (target) {
                if (is_enemy(gs, ai_idx, act.target)) {
                    score += 0.3;
                    score += target->hand_count * 0.2;
                } else {
                    score -= 0.6;
                }
            }
            break;
        case CARD_WU_ZHONG:
            score = w->wu_weight + (ai->hand_count < 4 ? 0.5 : 0.1);
            break;
        case CARD_SHAN:
            score = -1.0;
            break;
        default:
            score = 0.0;
            break;
    }

    if (ai->personality == PERSON_GAMBLER && act.action_type == 0) {
        score += random_double(-0.2, 0.2);
    }

    return score;
}

Action ai_decide_action(GameState* gs, int ai_idx) {
    Action act = {1, -1, -1};
    if (gs == NULL || ai_idx < 0 || ai_idx >= gs->player_count) {
        ai_generate_psych_message(gs, ai_idx, act);
        return act;
    }

    Character* ai = &gs->players[ai_idx];
    if (ai->hand_count <= 0) {
        ai_generate_psych_message(gs, ai_idx, act);
        return act;
    }

    Action actions[64];
    int action_count = game_get_legal_actions(gs, ai_idx, actions);
    if (action_count <= 0) {
        ai_generate_psych_message(gs, ai_idx, act);
        return act;
    }

    PersonalityWeights w = get_personality_weights(ai->personality);
    double best_score = -1e9;
    int best_index = 0;

    for (int i = 0; i < action_count; i++) {
        double score = score_action(gs, ai_idx, actions[i], &w);
        if (score > best_score) {
            best_score = score;
            best_index = i;
        }
    }

    act = actions[best_index];
    if (act.action_type == 1) {
        for (int i = 0; i < action_count; i++) {
            if (actions[i].action_type == 0) {
                int t = actions[i].target;
                int ci = actions[i].card_index;
                if (t < 0 || ci < 0 || ci >= ai->hand_count) continue;
                CardType ct = ai->hand[ci].type;
                if (ct == CARD_SHA && !is_enemy(gs, ai_idx, t)) continue;
                act = actions[i];
                break;
            }
        }
    }

    ai_generate_psych_message(gs, ai_idx, act);
    return act;
}

int ai_should_use_tao_to_save(GameState* gs, int saver_idx, int dying_idx) {
    if (gs == NULL || saver_idx < 0 || saver_idx >= gs->player_count ||
        dying_idx < 0 || dying_idx >= gs->player_count) {
        return 0;
    }

    Character* saver = &gs->players[saver_idx];
    Character* dying = &gs->players[dying_idx];
    if (!saver->is_ai) return 0;

    if (saver_idx == dying_idx) {
        return 1;
    }
    if (dying->camp != saver->camp) {
        return 0;
    }

    switch (saver->personality) {
        case PERSON_RADICAL:
            return 1;
        case PERSON_CONSERVATIVE:
            return 1;
        case PERSON_GAMBLER:
            return rand() % 2 == 0;
        default:
            return 0;
    }
}

/* ========================================================================
 * 闪响应决策：AI 被杀时决定是否出闪（含赵云龙胆）
 * ======================================================================== */
int ai_decide_shan_response(GameState* gs, int defender_idx,
                             int attacker_idx, int* out_card_idx,
                             int* out_use_sha_as_shan) {
    (void)attacker_idx;
    if (gs == NULL || defender_idx < 0 || defender_idx >= gs->player_count
        || out_card_idx == NULL || out_use_sha_as_shan == NULL)
        return 0;

    Character* defender = &gs->players[defender_idx];
    Personality p = defender->personality;
    int is_zhaoyun = (defender->hero == HERO_ZHAO_YUN);

    /* 查找可用的防御牌 */
    int shan_idx = -1, sha_idx = -1;
    for (int i = 0; i < defender->hand_count; i++) {
        if (defender->hand[i].type == CARD_SHAN) shan_idx = i;
        if (is_zhaoyun && defender->hand[i].type == CARD_SHA) sha_idx = i;
    }

    /* 无防御牌 → 承受伤害 */
    if (shan_idx < 0 && sha_idx < 0) {
        *out_card_idx = -1;
        *out_use_sha_as_shan = 0;
        return 0;
    }

    /* 濒死状态 → 根据人格决定 */
    if (defender->hp <= 1) {
        int will_use = 1;
        switch (p) {
            case PERSON_RADICAL:
                will_use = 0;  /* 激进：赌一波 */
                break;
            case PERSON_GAMBLER:
                will_use = (rand() % 2 == 0);  /* 赌徒：50% */
                break;
            case PERSON_CONSERVATIVE:
            default:
                will_use = 1;  /* 保守：必出闪 */
                break;
        }
        if (!will_use) {
            *out_card_idx = -1;
            *out_use_sha_as_shan = 0;
            return 0;
        }
    }

    *out_use_sha_as_shan = 0;

    /* 赵云：决定用闪还是用杀当闪 */
    if (is_zhaoyun) {
        if (shan_idx >= 0 && sha_idx >= 0) {
            /* 有闪有杀 → 人格决定 */
            switch (p) {
                case PERSON_RADICAL:
                    /* 激进：杀当闪，保留真闪（真闪更灵活） */
                    *out_use_sha_as_shan = 1;
                    *out_card_idx = sha_idx;
                    return 1;
                case PERSON_CONSERVATIVE:
                    /* 保守：真闪当闪，保留杀的进攻能力 */
                    *out_card_idx = shan_idx;
                    return 1;
                case PERSON_GAMBLER:
                default:
                    if (rand() % 2 == 0) {
                        *out_use_sha_as_shan = 1;
                        *out_card_idx = sha_idx;
                    } else {
                        *out_card_idx = shan_idx;
                    }
                    return 1;
            }
        } else if (shan_idx < 0 && sha_idx >= 0) {
            /* 只有杀 → 根据人格 */
            if (p == PERSON_CONSERVATIVE) {
                /* 保守：不出，留杀进攻 */
                *out_card_idx = -1;
                return 0;
            } else {
                *out_use_sha_as_shan = 1;
                *out_card_idx = sha_idx;
                return 1;
            }
        }
    }

    /* 普通武将或只有闪 → 用闪 */
    if (shan_idx >= 0) {
        *out_card_idx = shan_idx;
        return 1;
    }

    *out_card_idx = -1;
    return 0;
}

/* ========================================================================
 * 弃牌决策：弃牌阶段选择弃哪张
 * ======================================================================== */
int ai_decide_discard(GameState* gs, int ai_idx) {
    if (gs == NULL || ai_idx < 0 || ai_idx >= gs->player_count) return -1;

    Character* ch = &gs->players[ai_idx];
    Personality p = ch->personality;

    /* 特殊情况：过河拆桥但所有对手都没手牌 */
    for (int i = 0; i < ch->hand_count; i++) {
        if (ch->hand[i].type == CARD_GUO_CAI) {
            int can_use = 0;
            for (int t = 0; t < gs->player_count; t++) {
                if (t != ai_idx && gs->players[t].hp > 0
                    && gs->players[t].hand_count > 0) {
                    can_use = 1;
                    break;
                }
            }
            if (!can_use) return i;
        }
    }

    /* 统计各类牌数量 */
    int sha_count = 0, shan_count = 0;
    for (int i = 0; i < ch->hand_count; i++) {
        if (ch->hand[i].type == CARD_SHA) sha_count++;
        else if (ch->hand[i].type == CARD_SHAN) shan_count++;
    }

    int best_idx = -1, best_score = -9999;

    for (int i = 0; i < ch->hand_count; i++) {
        CardType t = ch->hand[i].type;
        int score = 0;

        if (t == CARD_SHA) {
            score = 20;
            if (sha_count > 2) score += 20;
            if (p == PERSON_RADICAL) score -= 15;
            if (p == PERSON_CONSERVATIVE) score += 10;
        }
        else if (t == CARD_SHAN) {
            score = 10;
            if (shan_count > 2) score += 15;
            if (p == PERSON_RADICAL) score += 20;
            if (p == PERSON_CONSERVATIVE) score -= 15;
        }
        else if (t == CARD_TAO) {
            if (ch->hp >= ch->max_hp) score = 80;
            else if (ch->hp <= 1) score = -100;
            else score = -20;
        }
        else if (t == CARD_GUO_CAI) {
            score = 80;
        }

        if (p == PERSON_GAMBLER)
            score += (rand() % 20) - 10;

        if (score > best_score) {
            best_score = score;
            best_idx = i;
        }
    }

    return best_idx;
}

/* ========================================================================
 * 观星排序决策：诸葛亮观星时决定牌堆顶顺序
 * ======================================================================== */
void ai_decide_star_order(GameState* gs, int ai_idx,
                          Card* cards, int count) {
    if (gs == NULL || ai_idx < 0 || ai_idx >= gs->player_count) return;
    if (count <= 1) return;

    Character* ch = &gs->players[ai_idx];
    Personality p = ch->personality;

    int scores[5];
    for (int i = 0; i < count; i++) {
        CardType t = cards[i].type;
        int s = 0;

        switch (t) {
            case CARD_SHA:   s = 7; break;
            case CARD_TAO:   s = 10; break;
            case CARD_SHAN:  s = 5; break;
            case CARD_WU_ZHONG: s = 8; break;
            case CARD_GUO_CAI: s = 6; break;
            default:         s = 3; break;
        }

        if (p == PERSON_RADICAL) {
            if (t == CARD_SHA) s += 3;
            if (t == CARD_SHAN) s -= 2;
        } else if (p == PERSON_CONSERVATIVE) {
            if (t == CARD_TAO) s += 2;
            if (t == CARD_SHAN) s += 2;
            if (t == CARD_SHA) s -= 2;
        } else if (p == PERSON_GAMBLER) {
            s += (rand() % 6) - 3;
        }

        scores[i] = s;
    }

    /* 冒泡排序：好牌放前面（自己先摸） */
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (scores[j] < scores[j + 1]) {
                int ts = scores[j];
                scores[j] = scores[j + 1];
                scores[j + 1] = ts;
                Card tc = cards[j];
                cards[j] = cards[j + 1];
                cards[j + 1] = tc;
            }
        }
    }
}

