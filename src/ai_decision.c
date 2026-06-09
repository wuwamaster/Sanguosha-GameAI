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

