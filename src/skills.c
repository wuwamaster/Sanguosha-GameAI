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

int skill_watch_stars(GameState* gs, int actor_idx) {
    if (gs == NULL) return 0;
    return gs->player_count;
}
