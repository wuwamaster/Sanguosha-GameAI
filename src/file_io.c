#include "file_io.h"
#include <stdio.h>
#include <string.h>

static CardType parse_card_name(const char* name) {
    if (strcmp(name, "SHA") == 0) return CARD_SHA;
    if (strcmp(name, "SHAN") == 0) return CARD_SHAN;
    if (strcmp(name, "TAO") == 0) return CARD_TAO;
    if (strcmp(name, "GUO_CAI") == 0) return CARD_GUO_CAI;
    if (strcmp(name, "WU_ZHONG") == 0) return CARD_WU_ZHONG;
    return CARD_SHA;
}

int load_card_deck(Card deck[], int max_cards) {
    if (deck == NULL || max_cards <= 0) return 0;

    FILE* file = fopen("data/cards.txt", "r");
    int count = 0;
    if (file != NULL) {
        char line[128];
        while (count < max_cards && fgets(line, sizeof(line), file)) {
            char* token = strtok(line, "\r\n");
            if (token == NULL) continue;
            deck[count].type = parse_card_name(token);
            deck[count].id = count;
            count++;
        }
        fclose(file);
    }

    if (count == 0) {
        deck[count++] = (Card){CARD_SHA, 0};
        if (count < max_cards) deck[count++] = (Card){CARD_SHAN, 1};
        if (count < max_cards) deck[count++] = (Card){CARD_TAO, 2};
    }
    return count;
}

int load_heroes(HeroType heroes[], int max_count) {
    if (heroes == NULL || max_count <= 0) return 0;
    int count = 0;
    heroes[count++] = HERO_ZHANG_FEI;
    if (count < max_count) heroes[count++] = HERO_ZHAO_YUN;
    if (count < max_count) heroes[count++] = HERO_ZHU_GE_LIANG;
    return count;
}

int load_personalities(Personality personalities[], int max_count) {
    if (personalities == NULL || max_count <= 0) return 0;
    int count = 0;
    personalities[count++] = PERSON_RADICAL;
    if (count < max_count) personalities[count++] = PERSON_CONSERVATIVE;
    if (count < max_count) personalities[count++] = PERSON_GAMBLER;
    return count;
}

void save_game_result(const char* mode_str, const char* player_hero, const char* ai_hero, const char* ai_personality, int winner) {
    FILE* file = fopen("game_results.txt", "a");
    if (file == NULL) return;
    fprintf(file, "mode=%s player=%s ai=%s personality=%s winner=%d\n",
            mode_str ? mode_str : "",
            player_hero ? player_hero : "",
            ai_hero ? ai_hero : "",
            ai_personality ? ai_personality : "",
            winner);
    fclose(file);
}
