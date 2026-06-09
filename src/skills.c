#include "game_engine.h"

int skill_can_use_sha(GameState* gs, int actor_idx) {
    if (gs == NULL || actor_idx < 0 || actor_idx >= gs->player_count) return 0;
    return gs->players[actor_idx].hero == HERO_ZHANG_FEI ? 1 : 0;
}

int skill_can_convert(GameState* gs, int actor_idx, CardType type) {
    if (gs == NULL || actor_idx < 0 || actor_idx >= gs->player_count) return 0;
    if (gs->players[actor_idx].hero != HERO_ZHAO_YUN) return 0;
    return type == CARD_SHA || type == CARD_SHAN;
}

int skill_watch_stars(GameState* gs, int actor_idx, Card* out_top, int* out_count) {
    if (gs == NULL || out_top == NULL || out_count == NULL) return 0;
    if (actor_idx < 0 || actor_idx >= gs->player_count) return 0;
    if (gs->players[actor_idx].hero != HERO_ZHU_GE_LIANG) {
        *out_count = 0;
        return 0;
    }

    int alive = 0;
    for (int i = 0; i < gs->player_count; i++)
        if (gs->players[i].hp > 0) alive++;

    int n = alive < 5 ? alive : 5;
    if (n > gs->pile_count) n = gs->pile_count;

    for (int i = 0; i < n; i++)
        out_top[i] = gs->draw_pile[gs->pile_count - 1 - i];

    *out_count = n;
    return n;
}

void skill_watch_stars_apply(GameState* gs, Card* new_order, int count) {
    if (gs == NULL || new_order == NULL || count <= 0 || count > gs->pile_count) return;

    Card* start = &gs->draw_pile[gs->pile_count - count];
    for (int i = 0; i < count; i++)
        start[i] = new_order[i];
}

int skill_empty_city_blocks_sha(GameState* gs, int target_idx) {
    if (gs == NULL) return 0;
    if (target_idx < 0 || target_idx >= gs->player_count) return 0;
    if (gs->players[target_idx].hero != HERO_ZHU_GE_LIANG) return 0;
    return gs->players[target_idx].hand_count == 0 ? 1 : 0;
}
