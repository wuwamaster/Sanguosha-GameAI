#include "file_io.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// ========== 对局记录查询接口 ==========

typedef struct {
    char mode[64];
    char player_hero[64];
    char ai_hero[64];
    char ai_personality[64];
    int winner;
} GameRecord;

// 读取所有游戏记录，返回记录数和动态分配的数组
int stats_load_records(GameRecord** out_records) {
    if (out_records == NULL) return 0;
    *out_records = NULL;

    // 先统计行数
    FILE* file = fopen("game_results.txt", "r");
    if (file == NULL) return 0;

    int capacity = 32;
    int count = 0;
    GameRecord* records = (GameRecord*)malloc(sizeof(GameRecord) * capacity);
    if (records == NULL) { fclose(file); return 0; }

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        if (count >= capacity) {
            capacity *= 2;
            GameRecord* tmp = (GameRecord*)realloc(records, sizeof(GameRecord) * capacity);
            if (tmp == NULL) break;
            records = tmp;
        }
        memset(&records[count], 0, sizeof(GameRecord));
        int parsed = sscanf(line, "mode=%63s player=%63s ai=%63s personality=%63s winner=%d",
                            records[count].mode,
                            records[count].player_hero,
                            records[count].ai_hero,
                            records[count].ai_personality,
                            &records[count].winner);
        if (parsed >= 5) {
            count++;
        }
    }
    fclose(file);
    *out_records = records;
    return count;
}

// 按玩家胜率排序（降序）
static int cmp_winrate_desc(const void* a, const void* b) {
    (void)a; (void)b;
    return 0;  // 简单实现：对于单条记录排序意义不大，保留接口
}

// 清除统计文件
void stats_clear_results(void) {
    FILE* file = fopen("game_results.txt", "w");
    if (file) fclose(file);
}

// 显示最近N条记录
void stats_show_recent(int n) {
    GameRecord* records = NULL;
    int count = stats_load_records(&records);
    if (count == 0) {
        printf("暂无对局记录。\n");
        return;
    }

    int start = (count > n) ? (count - n) : 0;
    printf("\n最近 %d 条对局记录:\n", count - start);
    printf("----------------------------------------\n");
    for (int i = start; i < count; i++) {
        const char* result = (records[i].winner == 0) ? "玩家胜" : "AI胜";
        printf("[%d] 模式:%s 玩家:%s AI:%s(%s) -> %s\n",
               i + 1,
               records[i].mode,
               records[i].player_hero,
               records[i].ai_hero,
               records[i].ai_personality,
               result);
    }
    printf("----------------------------------------\n");
    free(records);
}
