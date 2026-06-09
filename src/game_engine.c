#include "game_engine.h"
#include "ai.h"
#include <string.h>
#include <stdlib.h>

extern int skill_can_use_sha(GameState* gs, int actor_idx);//
extern int skill_can_convert(GameState* gs, int actor_idx, CardType type);
extern int skill_watch_stars(GameState* gs, int actor_idx, Card* out_top, int* out_count);
extern void skill_watch_stars_apply(GameState* gs, Card* new_order, int count);
extern int skill_empty_city_blocks_sha(GameState* gs, int target_idx);

static void remove_card_from_hand(Character* ch, int index) {
    if (ch == NULL || index < 0 || index >= ch->hand_count) return;
    for (int i = index; i < ch->hand_count - 1; i++) {
        ch->hand[i] = ch->hand[i + 1];
    }
    ch->hand_count--;
}

static void game_shuffle_deck(Card* deck, int count) {
    if (deck == NULL || count <= 1) return;
    for (int i = count - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Card tmp = deck[i];
        deck[i] = deck[j];
        deck[j] = tmp;
    }
}

static void game_init_deck(GameState* gs) {
    if (gs == NULL) return;
    gs->pile_count = 0;
    gs->discard_count = 0;

    for (int i = 0; i < 12 && gs->pile_count < MAX_PILE; i++)
        gs->draw_pile[gs->pile_count++] = (Card){CARD_SHA, gs->pile_count};
    for (int i = 0; i < 8 && gs->pile_count < MAX_PILE; i++)
        gs->draw_pile[gs->pile_count++] = (Card){CARD_SHAN, gs->pile_count};
    for (int i = 0; i < 5 && gs->pile_count < MAX_PILE; i++)
        gs->draw_pile[gs->pile_count++] = (Card){CARD_TAO, gs->pile_count};
    for (int i = 0; i < 3 && gs->pile_count < MAX_PILE; i++)
        gs->draw_pile[gs->pile_count++] = (Card){CARD_GUO_CAI, gs->pile_count};
    for (int i = 0; i < 2 && gs->pile_count < MAX_PILE; i++)
        gs->draw_pile[gs->pile_count++] = (Card){CARD_WU_ZHONG, gs->pile_count};

    game_shuffle_deck(gs->draw_pile, gs->pile_count);

    for (int p = 0; p < gs->player_count; p++)
        draw_card(gs, p, 4);
}

static int game_draw_from_pile(GameState* gs) {
    if (gs == NULL) return 0;

    if (gs->pile_count <= 0) {
        if (gs->discard_count <= 0) return 0;
        for (int i = 0; i < gs->discard_count && i < MAX_PILE; i++)
            gs->draw_pile[i] = gs->discard_pile[i];
        gs->pile_count = gs->discard_count;
        gs->discard_count = 0;
        game_shuffle_deck(gs->draw_pile, gs->pile_count);
    }

    gs->pile_count--;
    return 1;
}

void draw_card(GameState* gs, int char_idx, int num) {
    if (gs == NULL || char_idx < 0 || char_idx >= gs->player_count || num <= 0) return;

    Character* ch = &gs->players[char_idx];
    for (int i = 0; i < num; i++) {
        if (ch->hand_count >= MAX_HAND) break;
        if (game_draw_from_pile(gs)) {
            ch->hand[ch->hand_count++] = gs->draw_pile[gs->pile_count];
        } else {
            ch->hand[ch->hand_count++] = (Card){CARD_SHA, 0};
        }
    }
}

static void init_character(Character* ch, HeroType hero, Personality personality,
                           int is_ai, int is_lord, Camp camp) {
    ch->hero = hero;
    ch->personality = personality;
    ch->hp = 4;
    ch->max_hp = 4;
    ch->hand_count = 0;
    ch->is_ai = is_ai;
    ch->is_lord = is_lord;
    ch->camp = camp;
}

void game_init(GameState* gs, GameMode mode,
               int player_is_lord,
               HeroType player_hero,
               HeroType ai_hero1, Personality ai_person1,
               HeroType ai_hero2, Personality ai_person2) {
    if (gs == NULL) return;
    srand((unsigned)time(NULL));
    memset(gs, 0, sizeof(GameState));
    gs->mode = mode;
    gs->player_is_lord = player_is_lord;
    gs->player_count = 3;
    gs->current_turn = 0;
    gs->game_over = 0;
    gs->winner = 0;
    gs->turn_phase = 0;
    gs->sha_used_this_turn = 0;
    gs->need_shan_response = 0;
    gs->need_discard = 0;

    init_character(&gs->players[0], player_hero, PERSON_CONSERVATIVE, 0,
                   player_is_lord, CAMP_PLAYER);
    
    // 在主公局模式下，根据玩家身份设置AI阵营
    if (mode == MODE_LORD_VS_REBELS) {
        if (player_is_lord) {
            // 玩家是主公，两个AI都是反贼（敌方）
            init_character(&gs->players[1], ai_hero1, ai_person1, 1, 0,
                           CAMP_ENEMY);
            init_character(&gs->players[2], ai_hero2, ai_person2, 1, 0,
                           CAMP_ENEMY);
            gs->players[0].max_hp = 5;
            gs->players[0].hp = 5;
        } else {
            // 玩家是反贼，AI1是主公（敌方），AI2是反贼（队友）
            init_character(&gs->players[1], ai_hero1, ai_person1, 1, 1,
                           CAMP_ENEMY);
            init_character(&gs->players[2], ai_hero2, ai_person2, 1, 0,
                           CAMP_PLAYER);
            gs->players[1].max_hp = 5;
            gs->players[1].hp = 5;
        }
    } else {
        // 单挑模式，所有AI都是敌方
        init_character(&gs->players[1], ai_hero1, ai_person1, 1, 0,
                       CAMP_ENEMY);
        init_character(&gs->players[2], ai_hero2, ai_person2, 1, 0,
                       CAMP_ENEMY);
    }

    game_init_deck(gs);
}

static void game_check_death(GameState* gs, int idx) {
    if (gs == NULL || idx < 0 || idx >= gs->player_count) return;
    if (gs->players[idx].hp > 0) return;

    save_dying(gs, idx, gs->current_turn);
    if (gs->players[idx].hp <= 0) {
        if (gs->mode == MODE_SINGLE) {
            gs->game_over = 1;
            gs->winner = gs->players[idx].camp == CAMP_PLAYER ? 1 : 0;
            return;
        }
        if (gs->mode == MODE_LORD_VS_REBELS) {
            if (gs->players[idx].is_lord) {
                gs->game_over = 1;
                gs->winner = (gs->players[idx].camp == CAMP_PLAYER) ? 1 : 0;
                if (gs->winner != 0 && gs->winner != 1) gs->winner = 1;
                return;
            }
            int all_rebels_dead = 1;
            for (int i = 0; i < gs->player_count; i++) {
                if (!gs->players[i].is_lord && gs->players[i].hp > 0) {
                    all_rebels_dead = 0;
                    break;
                }
            }
            if (all_rebels_dead) {
                gs->game_over = 1;
                gs->winner = gs->players[gs->current_turn].camp == CAMP_PLAYER ? 0 : 1;
                int lord_camp = 0;
                for (int i = 0; i < gs->player_count; i++) {
                    if (gs->players[i].is_lord) {
                        lord_camp = gs->players[i].camp;
                        break;
                    }
                }
                gs->winner = (lord_camp == CAMP_PLAYER) ? 0 : 1;
            }
        }
    }
}

int save_dying(GameState* gs, int dying_idx, int start_idx) {
    if (gs == NULL || dying_idx < 0 || dying_idx >= gs->player_count) return 0;
    if (gs->players[dying_idx].hp > 0) return 1;

    int saved = 0;
    for (int tries = 0; tries < gs->player_count && gs->players[dying_idx].hp <= 0; tries++) {
        int searcher = (start_idx + tries) % gs->player_count;

        int has_tao = 0;
        int tao_idx = -1;
        Character* saver = &gs->players[searcher];
        for (int i = 0; i < saver->hand_count; i++) {
            if (saver->hand[i].type == CARD_TAO) {
                has_tao = 1;
                tao_idx = i;
                break;
            }
        }

        int use_tao = 0;
        if (has_tao) {
            if (saver->is_ai) {
                use_tao = ai_should_use_tao_to_save(gs, searcher, dying_idx);
            } else {
                use_tao = 1;
            }
        }

        if (use_tao && tao_idx >= 0) {
            remove_card_from_hand(saver, tao_idx);
            gs->discard_pile[gs->discard_count++] = (Card){CARD_TAO, 0};
            gs->players[dying_idx].hp++;
            saved = 1;
        }
    }

    return saved;
}

void apply_card_effect(GameState* gs, int user_idx, Card card, int target_idx) {
    if (gs == NULL || user_idx < 0 || user_idx >= gs->player_count ||
        target_idx < 0 || target_idx >= gs->player_count) {
        return;
    }

    Character* target = &gs->players[target_idx];
    switch (card.type) {
        case CARD_SHA:
            if (target->hp > 0) target->hp -= 1;
            game_check_death(gs, target_idx);
            break;
        case CARD_TAO:
            target->hp += 1;
            if (target->hp > target->max_hp) target->hp = target->max_hp;
            break;
        case CARD_GUO_CAI:
            if (target->hand_count > 0)
                remove_card_from_hand(target, 0);
            break;
        case CARD_WU_ZHONG:
            draw_card(gs, target_idx, 2);
            break;
        default:
            break;
    }
}

static int card_target_is_legal(GameState* gs, int actor_idx, CardType type, int target_idx) {
    if (gs == NULL || target_idx < 0 || target_idx >= gs->player_count)
        return 0;
    if (gs->players[target_idx].hp <= 0)
        return 0;
    if (type == CARD_TAO) {
        return target_idx == actor_idx;
    }
    if (type == CARD_WU_ZHONG) {
        return target_idx == actor_idx;
    }
    if (type == CARD_SHA) {
        if (target_idx == actor_idx) return 0;
        if (skill_empty_city_blocks_sha(gs, target_idx)) return 0;
    }
    return 1;
}

static int can_use_more_sha(GameState* gs, int actor_idx) {
    if (gs == NULL) return 0;
    if (gs->players[actor_idx].hero == HERO_ZHANG_FEI) return 1;
    return gs->sha_used_this_turn < 1;
}

int game_get_legal_actions(GameState* gs, int actor_idx, Action* out_actions) {
    if (gs == NULL || out_actions == NULL || actor_idx < 0 || actor_idx >= gs->player_count)
        return 0;

    int count = 0;
    Character* actor = &gs->players[actor_idx];
    int is_zhaoyun = (actor->hero == HERO_ZHAO_YUN);

    for (int i = 0; i < actor->hand_count && count < MAX_HAND; i++) {
        CardType type = actor->hand[i].type;
        int can_act_as_sha = (type == CARD_SHA) ||
            (is_zhaoyun && type == CARD_SHAN);

        if (can_act_as_sha) {
            if (!can_use_more_sha(gs, actor_idx)) continue;
            int found = 0;
            for (int t = 0; t < gs->player_count; t++) {
                if (card_target_is_legal(gs, actor_idx, CARD_SHA, t)) {
                    out_actions[count].action_type = 0;
                    out_actions[count].card_index = i;
                    out_actions[count].target = t;
                    count++;
                    found = 1;
                }
            }
            if (!found) continue;
        } else if (type == CARD_TAO) {
            if (card_target_is_legal(gs, actor_idx, type, actor_idx)) {
                out_actions[count].action_type = 0;
                out_actions[count].card_index = i;
                out_actions[count].target = actor_idx;
                count++;
            }
        } else {
            for (int t = 0; t < gs->player_count; t++) {
                if (card_target_is_legal(gs, actor_idx, type, t)) {
                    out_actions[count].action_type = 0;
                    out_actions[count].card_index = i;
                    out_actions[count].target = t;
                    count++;
                }
            }
        }
    }

    out_actions[count].action_type = 1;
    out_actions[count].card_index = -1;
    out_actions[count].target = -1;
    count++;

    return count;
}

ActionResult game_perform_action(GameState* gs, Action act) {
    ActionResult res = {0, 0, 0};
    if (gs == NULL) return res;

    if (act.action_type == 1) {
        gs->turn_phase = 4;
        res.success = 1;
        res.game_over = gs->game_over;
        res.winner = gs->winner;
        return res;
    }

    if (act.action_type == 0) {
        if (gs->current_turn < 0 || gs->current_turn >= gs->player_count) return res;
        Character* actor = &gs->players[gs->current_turn];

        if (act.card_index < 0 || act.card_index >= actor->hand_count) return res;
        if (act.target < 0 || act.target >= gs->player_count) return res;
        if (gs->players[act.target].hp <= 0) return res;

        Card card = actor->hand[act.card_index];
        CardType effective_type = card.type;

        if (actor->hero == HERO_ZHAO_YUN) {
            if (card.type == CARD_SHAN && act.target != gs->current_turn)
                effective_type = CARD_SHA;
        }

        if (!card_target_is_legal(gs, gs->current_turn, effective_type, act.target)) return res;

        if (effective_type == CARD_SHA && !can_use_more_sha(gs, gs->current_turn)) return res;

        remove_card_from_hand(actor, act.card_index);
        gs->discard_pile[gs->discard_count++] = card;

        if (effective_type == CARD_SHA) {
            gs->sha_used_this_turn++;
            gs->need_shan_response = 1;
            gs->shan_source = gs->current_turn;
            gs->shan_target = act.target;
            res.success = 1;
        } else {
            apply_card_effect(gs, gs->current_turn, card, act.target);
            res.success = 1;
        }
    }

    res.game_over = gs->game_over;
    res.winner = gs->winner;
    return res;
}

int game_resolve_shan(GameState* gs, int shan_card_idx) {
    if (gs == NULL || !gs->need_shan_response) return 0;

    Character* target = &gs->players[gs->shan_target];
    int blocked = 0;

    int is_zhaoyun = (target->hero == HERO_ZHAO_YUN);

    if (shan_card_idx >= 0 && shan_card_idx < target->hand_count) {
        CardType rt = target->hand[shan_card_idx].type;
        if (rt == CARD_SHAN || (is_zhaoyun && rt == CARD_SHA)) {
            Card discard_entry = {rt, 0};
            remove_card_from_hand(target, shan_card_idx);
            gs->discard_pile[gs->discard_count++] = discard_entry;
            blocked = 1;
        }
    }


    if (!blocked) {
        apply_card_effect(gs, gs->shan_source,
                          (Card){CARD_SHA, 0}, gs->shan_target);
    }

    gs->need_shan_response = 0;
    gs->shan_source = -1;
    gs->shan_target = -1;

    return blocked ? 2 : 1;
}

int game_is_turn_over(GameState* gs) {
    if (gs == NULL) return 1;
    if (gs->game_over) return 1;
    if (gs->need_shan_response) return 0;
    if (gs->need_discard) return 0;
    return gs->turn_phase == 4 ? 1 : 0;
}

static void game_perform_turn_switch(GameState* gs) {
    if (gs == NULL) return;

    int next = gs->current_turn;
    int found = 0;
    for (int tries = 0; tries < gs->player_count; tries++) {
        next = (next + 1) % gs->player_count;
        if (gs->players[next].hp > 0) {
            found = 1;
            break;
        }
    }
    if (!found) return;

    gs->current_turn = next;
    gs->sha_used_this_turn = 0;
    gs->turn_phase = 0;
}

int game_advance_phase(GameState* gs) {
    if (gs == NULL) return 0;

    if (gs->turn_phase == 0) {
        if (gs->players[gs->current_turn].hero == HERO_ZHU_GE_LIANG) {
            Card top[5];
            int top_count = 0;
            skill_watch_stars(gs, gs->current_turn, top, &top_count);
            if (top_count > 0) {
                if (!gs->players[gs->current_turn].is_ai) {
                    gs->need_star_choice = 1;
                    gs->star_watch_count = top_count;
                    for (int i = 0; i < top_count; i++) {
                        gs->star_watch_cards[i] = top[i];
                        gs->star_current_slots[i] = i;
                    }
                    return 0;
                }
                for (int i = 0; i < top_count - 1; i++) {
                    int worst = i;
                    for (int j = i + 1; j < top_count; j++)
                        if (top[j].type < top[worst].type)
                            worst = j;
                    Card tmp = top[i];
                    top[i] = top[worst];
                    top[worst] = tmp;
                }
                skill_watch_stars_apply(gs, top, top_count);
            }
        }
        gs->turn_phase = 1;
        return 1;
    }

    if (gs->turn_phase == 1) {
        int draw_count = 2;
        if (gs->mode == MODE_LORD_VS_REBELS && gs->players[gs->current_turn].is_lord)
            draw_count = 3;
        draw_card(gs, gs->current_turn, draw_count);
        gs->turn_phase = 2;
        return 1;
    }

    return 0;
}

void game_next_turn(GameState* gs) {
    if (gs == NULL) return;

    Character* cur = &gs->players[gs->current_turn];
    int excess = cur->hand_count - cur->hp;

    if (excess > 0 && cur->hp > 0) {
        if (!cur->is_ai) {
            gs->need_discard = 1;
            gs->turn_phase = 3;
            return;
        } else {
            for (int i = 0; i < excess && cur->hand_count > 0; i++) {
                gs->discard_pile[gs->discard_count++] = cur->hand[0];
                remove_card_from_hand(cur, 0);
            }
        }
    }

    gs->need_discard = 0;
    game_perform_turn_switch(gs);
    while (gs->turn_phase < 2 && !gs->need_star_choice) game_advance_phase(gs);
}

void game_discard_card(GameState* gs, int card_idx) {
    if (gs == NULL || !gs->need_discard) return;
    Character* cur = &gs->players[gs->current_turn];
    if (card_idx < 0 || card_idx >= cur->hand_count) return;

    gs->discard_pile[gs->discard_count++] = cur->hand[card_idx];
    remove_card_from_hand(cur, card_idx);
}

void game_confirm_discard_done(GameState* gs) {
    if (gs == NULL || !gs->need_discard) return;

    Character* cur = &gs->players[gs->current_turn];
    if (cur->hand_count <= cur->hp) {
        gs->need_discard = 0;
        game_perform_turn_switch(gs);
        while (gs->turn_phase < 2 && !gs->need_star_choice) game_advance_phase(gs);
    }
}

void game_swap_star_slots(GameState* gs, int slot_a, int slot_b) {
    if (gs == NULL || !gs->need_star_choice) return;
    if (slot_a < 0 || slot_a >= gs->star_watch_count) return;
    if (slot_b < 0 || slot_b >= gs->star_watch_count) return;
    int tmp = gs->star_current_slots[slot_a];
    gs->star_current_slots[slot_a] = gs->star_current_slots[slot_b];
    gs->star_current_slots[slot_b] = tmp;
}

void game_confirm_star_choice(GameState* gs) {
    if (gs == NULL || !gs->need_star_choice) return;

    Card order[5];
    for (int i = 0; i < gs->star_watch_count; i++)
        order[i] = gs->star_watch_cards[gs->star_current_slots[i]];

    skill_watch_stars_apply(gs, order, gs->star_watch_count);
    gs->need_star_choice = 0;
    gs->star_watch_count = 0;
    gs->turn_phase = 1;
}
