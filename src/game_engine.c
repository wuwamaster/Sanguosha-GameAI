#include "game_engine.h"
#include <string.h>

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
    ch->hand[0].type = 0;
}

void game_init(GameState* gs, GameMode mode,
               int player_is_lord,
               HeroType player_hero,
               HeroType ai_hero1, Personality ai_person1,
               HeroType ai_hero2, Personality ai_person2) {
    if (gs == NULL) return;
    memset(gs, 0, sizeof(GameState));
    gs->mode = mode;
    gs->player_is_lord = player_is_lord;
    gs->player_count = 3;
    gs->current_turn = 0;
    gs->game_over = 0;
    gs->winner = 0;

    init_character(&gs->players[0], player_hero, PERSON_CONSERVATIVE, 0,
                   player_is_lord, CAMP_PLAYER);
    init_character(&gs->players[1], ai_hero1, ai_person1, 1, 0,
                   CAMP_ENEMY);
    init_character(&gs->players[2], ai_hero2, ai_person2, 1, 0,
                   CAMP_ENEMY);
}

ActionResult game_perform_action(GameState* gs, Action act) {
    ActionResult res = {0,0,0};
    if (gs == NULL) return res;
    if (act.action_type == 0 && act.target >= 0 && act.target < gs->player_count) {
        if (gs->current_turn >= 0 && gs->current_turn < gs->player_count &&
            act.card_index >= 0 && act.card_index < gs->players[gs->current_turn].hand_count) {
            Card card = gs->players[gs->current_turn].hand[act.card_index];
            apply_card_effect(gs, gs->current_turn, card, act.target);
            res.success = 1;
        }
    } else if (act.action_type == 1) {
        res.success = 1;
    }
    res.game_over = gs->game_over;
    res.winner = gs->winner;
    return res;
}

int game_is_turn_over(GameState* gs) {
    if (gs == NULL) return 1;
    return gs->game_over ? 1 : 0;
}

void game_next_turn(GameState* gs) {
    if (gs == NULL) return;
    gs->current_turn = (gs->current_turn + 1) % gs->player_count;
}

int game_get_legal_actions(GameState* gs, int actor_idx, Action* out_actions) {
    if (gs == NULL || out_actions == NULL || actor_idx < 0 || actor_idx >= gs->player_count) {
        return 0;
    }
    out_actions[0].action_type = 1;
    out_actions[0].card_index = -1;
    out_actions[0].target = -1;
    return 1;
}

void draw_card(GameState* gs, int char_idx, int num) {
    if (gs == NULL || char_idx < 0 || char_idx >= gs->player_count || num <= 0) return;
    Character* ch = &gs->players[char_idx];
    int draw = num;
    if (ch->hand_count + draw > MAX_HAND) {
        draw = MAX_HAND - ch->hand_count;
    }
    for (int i = 0; i < draw; i++) {
        ch->hand[ch->hand_count++].type = CARD_WU_ZHONG;
    }
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
            break;
        case CARD_TAO:
            target->hp += 1;
            if (target->hp > target->max_hp) target->hp = target->max_hp;
            break;
        case CARD_GUO_CAI:
            if (target->hand_count > 0) target->hand_count -= 1;
            break;
        case CARD_WU_ZHONG:
            draw_card(gs, target_idx, 2);
            break;
        default:
            break;
    }
}

int save_dying(GameState* gs, int dying_idx, int start_idx) {
    (void)gs;
    (void)dying_idx;
    (void)start_idx;
    return 0;
}
