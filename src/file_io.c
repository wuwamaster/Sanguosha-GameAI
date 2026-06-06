#include "file_io.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// ========== 内部辅助函数 ==========

static CardType parse_card_name(const char* name) {
    if (strcmp(name, "SHA") == 0) return CARD_SHA;
    if (strcmp(name, "SHAN") == 0) return CARD_SHAN;
    if (strcmp(name, "TAO") == 0) return CARD_TAO;
    if (strcmp(name, "GUO_CAI") == 0) return CARD_GUO_CAI;
    if (strcmp(name, "WU_ZHONG") == 0) return CARD_WU_ZHONG;
    return CARD_SHA;
}

static HeroType parse_hero_name(const char* name) {
    if (strcmp(name, "ZHANG_FEI") == 0) return HERO_ZHANG_FEI;
    if (strcmp(name, "ZHAO_YUN") == 0) return HERO_ZHAO_YUN;
    if (strcmp(name, "ZHU_GE_LIANG") == 0) return HERO_ZHU_GE_LIANG;
    return HERO_ZHANG_FEI;
}

static const char* hero_to_string(HeroType hero) {
    switch (hero) {
        case HERO_ZHANG_FEI:    return "ZHANG_FEI";
        case HERO_ZHAO_YUN:     return "ZHAO_YUN";
        case HERO_ZHU_GE_LIANG: return "ZHU_GE_LIANG";
        default: return "UNKNOWN";
    }
}

static const char* personality_to_string(Personality p) {
    switch (p) {
        case PERSON_RADICAL:      return "RADICAL";
        case PERSON_CONSERVATIVE: return "CONSERVATIVE";
        case PERSON_GAMBLER:      return "GAMBLER";
        default: return "UNKNOWN";
    }
}

static const char* mode_to_string(GameMode m) {
    switch (m) {
        case MODE_SINGLE:          return "SINGLE";
        case MODE_LORD_VS_REBELS:  return "LORD_VS_REBELS";
        default: return "UNKNOWN";
    }
}

static Personality parse_personality_name(const char* name) {
    if (strcmp(name, "RADICAL") == 0) return PERSON_RADICAL;
    if (strcmp(name, "CONSERVATIVE") == 0) return PERSON_CONSERVATIVE;
    if (strcmp(name, "GAMBLER") == 0) return PERSON_GAMBLER;
    return PERSON_RADICAL;
}

// ========== 牌堆加载 ==========

int load_card_deck(Card deck[], int max_cards) {
    if (deck == NULL || max_cards <= 0) return 0;

    FILE* file = fopen("data/cards.txt", "r");
    int count = 0;
    if (file != NULL) {
        char line[128];
        while (count < max_cards && fgets(line, sizeof(line), file)) {
            // 去除换行符
            size_t len = strlen(line);
            while (len > 0 && (line[len-1] == '\r' || line[len-1] == '\n')) {
                line[--len] = '\0';
            }
            if (line[0] == '\0' || line[0] == '#') continue;  // 跳过空行和注释
            deck[count].type = parse_card_name(line);
            deck[count].id = count;
            count++;
        }
        fclose(file);
    }

    // 如果文件为空或无法读取，使用默认牌堆
    if (count == 0) {
        // 12张杀
        for (int i = 0; i < 12 && count < max_cards; i++)
            deck[count++] = (Card){CARD_SHA, count};
        // 8张闪
        for (int i = 0; i < 8 && count < max_cards; i++)
            deck[count++] = (Card){CARD_SHAN, count};
        // 5张桃
        for (int i = 0; i < 5 && count < max_cards; i++)
            deck[count++] = (Card){CARD_TAO, count};
        // 3张过河拆桥
        for (int i = 0; i < 3 && count < max_cards; i++)
            deck[count++] = (Card){CARD_GUO_CAI, count};
        // 2张无中生有
        for (int i = 0; i < 2 && count < max_cards; i++)
            deck[count++] = (Card){CARD_WU_ZHONG, count};
    }
    return count;
}

// ========== 武将加载 ==========

int load_heroes(HeroType heroes[], int max_count) {
    if (heroes == NULL || max_count <= 0) return 0;

    FILE* file = fopen("data/heroes.txt", "r");
    int count = 0;
    if (file != NULL) {
        char line[128];
        while (count < max_count && fgets(line, sizeof(line), file)) {
            size_t len = strlen(line);
            while (len > 0 && (line[len-1] == '\r' || line[len-1] == '\n')) {
                line[--len] = '\0';
            }
            if (line[0] == '\0' || line[0] == '#') continue;
            heroes[count] = parse_hero_name(line);
            count++;
        }
        fclose(file);
    }

    // 默认3武将
    if (count == 0) {
        heroes[count++] = HERO_ZHANG_FEI;
        if (count < max_count) heroes[count++] = HERO_ZHAO_YUN;
        if (count < max_count) heroes[count++] = HERO_ZHU_GE_LIANG;
    }
    return count;
}

// ========== 人格加载 ==========

int load_personalities(PersonalityWeights weights[], int max_count) {
    if (weights == NULL || max_count <= 0) return 0;

    FILE* file = fopen("data/personalities.txt", "r");
    int count = 0;
    if (file != NULL) {
        char line[256];
        while (count < max_count && fgets(line, sizeof(line), file)) {
            size_t len = strlen(line);
            while (len > 0 && (line[len-1] == '\r' || line[len-1] == '\n')) {
                line[--len] = '\0';
            }
            if (line[0] == '\0' || line[0] == '#') continue;

            // 解析: NAME sha_weight shan_weight tao_weight guo_weight wu_weight aggressiveness hp_threshold
            char name[32] = {0};
            double sha_w, shan_w, tao_w, guo_w, wu_w, agg;
            int hp_th;
            int parsed = sscanf(line, "%31s %lf %lf %lf %lf %lf %lf %d",
                                name, &sha_w, &shan_w, &tao_w, &guo_w, &wu_w, &agg, &hp_th);
            if (parsed >= 8) {
                weights[count].personality = parse_personality_name(name);
                strncpy(weights[count].name, name, sizeof(weights[count].name) - 1);
                weights[count].sha_weight = sha_w;
                weights[count].shan_weight = shan_w;
                weights[count].tao_weight = tao_w;
                weights[count].guo_weight = guo_w;
                weights[count].wu_weight = wu_w;
                weights[count].aggressiveness = agg;
                weights[count].hp_threshold = hp_th;
                count++;
            }
        }
        fclose(file);
    }

    // 默认人格权重
    if (count == 0) {
        weights[count++] = (PersonalityWeights){PERSON_RADICAL,      1.5, 0.5, 0.6, 1.3, 1.4, 0.85, 2, "RADICAL"};
        if (count < max_count)
            weights[count++] = (PersonalityWeights){PERSON_CONSERVATIVE, 0.5, 1.5, 1.5, 0.7, 0.6, 0.25, 3, "CONSERVATIVE"};
        if (count < max_count)
            weights[count++] = (PersonalityWeights){PERSON_GAMBLER,      1.0, 1.0, 1.0, 1.8, 1.8, 0.50, 2, "GAMBLER"};
    }
    return count;
}

int load_personality_types(Personality personalities[], int max_count) {
    if (personalities == NULL || max_count <= 0) return 0;
    int count = 0;
    personalities[count++] = PERSON_RADICAL;
    if (count < max_count) personalities[count++] = PERSON_CONSERVATIVE;
    if (count < max_count) personalities[count++] = PERSON_GAMBLER;
    return count;
}

// ========== 游戏结果保存 ==========

void save_game_result(const char* mode_str,
                      const char* player_hero,
                      const char* ai_hero,
                      const char* ai_personality,
                      int winner) {
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

// ========== 统计功能 ==========

Stats get_stats(void) {
    Stats s;
    memset(&s, 0, sizeof(s));

    FILE* file = fopen("game_results.txt", "r");
    if (file == NULL) return s;

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        char mode[64] = {0}, player[64] = {0}, ai[64] = {0}, personality[64] = {0};
        int winner = 0;

        int parsed = sscanf(line, "mode=%63s player=%63s ai=%63s personality=%63s winner=%d",
                            mode, player, ai, personality, &winner);
        if (parsed < 5) continue;

        s.total_games++;

        // 按胜负统计
        if (winner == 0) s.player_wins++;
        else s.ai_wins++;

        // 按武将统计
        HeroType ph = parse_hero_name(player);
        s.hero_games[ph]++;
        if (winner == 0) s.hero_wins[ph]++;

        // 按模式统计
        if (strcmp(mode, "SINGLE") == 0) {
            s.mode_games[0]++;
            if (winner == 0) s.mode_wins[0]++;
        } else if (strcmp(mode, "LORD_VS_REBELS") == 0) {
            s.mode_games[1]++;
            if (winner == 0) s.mode_wins[1]++;
        }

        // 按人格统计
        Personality pp = parse_personality_name(personality);
        s.personality_games[pp]++;
        if (winner == 0) s.personality_wins[pp]++;
    }
    fclose(file);
    return s;
}

void stats_print(const Stats* s) {
    if (s == NULL) return;

    printf("\n");
    printf("========================================\n");
    printf("           游戏统计数据\n");
    printf("========================================\n");
    printf("  总局数: %d\n", s->total_games);
    if (s->total_games > 0) {
        printf("  玩家胜: %d (%.1f%%)\n", s->player_wins,
               100.0 * s->player_wins / s->total_games);
        printf("  AI  胜: %d (%.1f%%)\n", s->ai_wins,
               100.0 * s->ai_wins / s->total_games);
    } else {
        printf("  暂无对局记录\n");
    }

    // 按武将
    printf("----------------------------------------\n");
    printf("  武将胜率:\n");
    const char* hero_names[] = {"张飞", "赵云", "诸葛亮"};
    for (int i = 0; i < 3; i++) {
        int games = s->hero_games[i];
        if (games > 0) {
            printf("    %s: %d/%d (%.1f%%)\n", hero_names[i],
                   s->hero_wins[i], games,
                   100.0 * s->hero_wins[i] / games);
        } else {
            printf("    %s: 无数据\n", hero_names[i]);
        }
    }

    // 按模式
    printf("----------------------------------------\n");
    printf("  模式胜率:\n");
    const char* mode_names[] = {"单挑", "主公局"};
    for (int i = 0; i < 2; i++) {
        int games = s->mode_games[i];
        if (games > 0) {
            printf("    %s: %d/%d (%.1f%%)\n", mode_names[i],
                   s->mode_wins[i], games,
                   100.0 * s->mode_wins[i] / games);
        } else {
            printf("    %s: 无数据\n", mode_names[i]);
        }
    }

    // 按人格
    printf("----------------------------------------\n");
    printf("  对AI人格胜率:\n");
    const char* personality_names[] = {"激进派", "保守派", "赌徒派"};
    for (int i = 0; i < 3; i++) {
        int games = s->personality_games[i];
        if (games > 0) {
            printf("    %s: %d/%d (%.1f%%)\n", personality_names[i],
                   s->personality_wins[i], games,
                   100.0 * s->personality_wins[i] / games);
        } else {
            printf("    %s: 无数据\n", personality_names[i]);
        }
    }
    printf("========================================\n\n");
}

// ========== 后台管理（保存配置）==========

int save_heroes(const char* hero_names[], int count) {
    if (hero_names == NULL || count <= 0) return 0;
    FILE* file = fopen("data/heroes.txt", "w");
    if (file == NULL) return 0;
    for (int i = 0; i < count; i++) {
        fprintf(file, "%s\n", hero_names[i] ? hero_names[i] : "ZHANG_FEI");
    }
    fclose(file);
    return 1;
}

int save_personalities(const PersonalityWeights weights[], int count) {
    if (weights == NULL || count <= 0) return 0;
    FILE* file = fopen("data/personalities.txt", "w");
    if (file == NULL) return 0;
    fprintf(file, "# 人格参数配置文件\n");
    fprintf(file, "# 格式：名称 杀权重 闪权重 桃权重 过拆权重 无中权重 激进程度(0-1) 吃桃血量阈值\n");
    for (int i = 0; i < count; i++) {
        fprintf(file, "%s %.2f %.2f %.2f %.2f %.2f %.2f %d\n",
                personality_to_string(weights[i].personality),
                weights[i].sha_weight,
                weights[i].shan_weight,
                weights[i].tao_weight,
                weights[i].guo_weight,
                weights[i].wu_weight,
                weights[i].aggressiveness,
                weights[i].hp_threshold);
    }
    fclose(file);
    return 1;
}
