/*
 * src/ui_raylib.c - raylib 图形界面实现
 * 与 src/ui_console.c 同接口，主程序用本文件，测试用 console 版。
 *
 * M3：鼠标交互（手牌点击、目标点击、结束回合按钮、闪/弃牌弹窗）
 */

#include "ui.h"
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------- 窗口与字体 ---------- */
#define WIN_W 1024
#define WIN_H 640
#define FONT_BASE_SIZE 28

static int  g_window_ready = 0;
static Font g_font;
static int  g_font_loaded = 0;

static char   g_psych_msg[256] = {0};
static int    g_psych_emotion  = 0;
static double g_psych_until    = 0.0;

/* ---------- 出牌区（最近 N 次出牌） ---------- */
#define RECENT_PLAY_MAX 4
typedef struct {
    int      actor;       /* 0=玩家, 1/2=AI */
    int      target;      /* 同上；-1 表示无目标 */
    Card     card;
    double   shown_at;
} RecentPlay;
static RecentPlay g_recent[RECENT_PLAY_MAX];
static int        g_recent_count = 0;
#define RECENT_VISIBLE_SEC 2.5

/* ---------- 浮动提示系统 ---------- */
typedef struct {
    char text[128];
    double start_time;
    float duration;
    Color color;
} FloatingHint;

static FloatingHint g_floating_hint = {0};

static void show_floating_hint(const char* text, Color color, float duration) {
    strncpy(g_floating_hint.text, text, sizeof(g_floating_hint.text)-1);
    g_floating_hint.start_time = GetTime();
    g_floating_hint.duration = duration;
    g_floating_hint.color = color;
}

/* draw_floating_hint 的实现放在 DrawCN/MeasureCN 定义之后 */

/* ---------- 事件日志 ---------- */
static void draw_events(GameState* gs);

/* ---------- 名称辅助 ---------- */
static const char* hero_name(HeroType h) {
    switch (h) {
        case HERO_ZHANG_FEI:    return "张飞";
        case HERO_ZHAO_YUN:     return "赵云";
        case HERO_ZHU_GE_LIANG: return "诸葛亮";
        default: return "?";
    }
}
static const char* card_name(CardType t) {
    switch (t) {
        case CARD_SHA:      return "杀";
        case CARD_SHAN:     return "闪";
        case CARD_TAO:      return "桃";
        case CARD_GUO_CAI:  return "过河拆桥";
        case CARD_WU_ZHONG: return "无中生有";
        default: return "?";
    }
}
static const char* person_name(Personality p) {
    switch (p) {
        case PERSON_RADICAL:      return "激进派";
        case PERSON_CONSERVATIVE: return "保守派";
        case PERSON_GAMBLER:      return "赌徒派";
        default: return "?";
    }
}
static const char* phase_name(int phase) {
    switch (phase) {
        case 0: return "准备"; case 1: return "摸牌";
        case 2: return "出牌"; case 3: return "弃牌";
        case 4: return "结束"; default: return "?";
    }
}

/* ---------- 中文绘制 ---------- */
static void DrawCN(const char* text, int x, int y, int fs, Color color) {
    if (!g_font_loaded) { DrawText(text, x, y, fs, color); return; }
    DrawTextEx(g_font, text, (Vector2){(float)x,(float)y}, (float)fs, 1.0f, color);
}
static int MeasureCN(const char* text, int fs) {
    if (!g_font_loaded) return MeasureText(text, fs);
    return (int)MeasureTextEx(g_font, text, (float)fs, 1.0f).x;
}

/* ---------- 浮动提示绘制 ---------- */
static void draw_floating_hint(void) {
    if (g_floating_hint.text[0] == 0) return;
    
    double elapsed = GetTime() - g_floating_hint.start_time;
    if (elapsed > g_floating_hint.duration) {
        g_floating_hint.text[0] = 0;
        return;
    }
    
    // 淡入淡出
    float alpha = 1.0f;
    if (elapsed < 0.2f) alpha = elapsed / 0.2f;
    if (elapsed > g_floating_hint.duration - 0.5f) 
        alpha = (g_floating_hint.duration - elapsed) / 0.5f;
    
    // 从下方滑入
    int y_offset = (int)((1.0f - alpha) * 30);
    
    Color c = g_floating_hint.color;
    c.a = (unsigned char)(255 * alpha);
    
    int tw = MeasureCN(g_floating_hint.text, 24);
    int x = (WIN_W - tw) / 2;
    int y = 160 + y_offset;
    
    // 阴影 + 主文字
    DrawCN(g_floating_hint.text, x+2, y+2, 24, (Color){0,0,0,(unsigned char)(100*alpha)});
    DrawCN(g_floating_hint.text, x, y, 24, c);
}

/* ---------- 布局参数（绘制与点击共享） ---------- */
#define HAND_X 40
#define HAND_Y 485
#define CARD_W 90
#define CARD_H 120
#define CARD_GAP 10
#define AI_X0 40
#define AI_Y  30
#define AI_W  280
#define AI_H  150
#define AI_GAP 300
#define END_BTN_X 880
#define END_BTN_Y 595
#define END_BTN_W 130
#define END_BTN_H 38

static Rectangle card_rect(int i) {
    return (Rectangle){ HAND_X + i * (CARD_W + CARD_GAP), HAND_Y, CARD_W, CARD_H };
}
static Rectangle ai_rect(int i /* 1.. */) {
    return (Rectangle){ AI_X0 + (i-1) * AI_GAP, AI_Y, AI_W, AI_H };
}
static Rectangle player_area_rect(void) { return (Rectangle){20, 380, WIN_W-40, 240}; }
static Rectangle end_btn_rect(void)     { return (Rectangle){END_BTN_X, END_BTN_Y, END_BTN_W, END_BTN_H}; }

/* ---------- 字体加载 ----------
 * 策略：覆盖 ASCII + CJK 统一表意文字 + 中文常用标点 + 圈号等。
 * 这样 AI 动态生成的心理文字、菜单文案的任意常用中文都能正常显示，
 * 不再需要维护 CN_CHARS 白名单。
 */
static int* build_codepoints(int* out_total) {
    /* 范围列表：[start, end] 闭区间 */
    static const int ranges[][2] = {
        {0x0020, 0x007E},   /* ASCII 可见 */
        {0x00A0, 0x00FF},   /* Latin-1 补充 */
        {0x2000, 0x206F},   /* 通用标点 —— 、…等 */
        {0x2070, 0x209F},   /* 上下标 */
        {0x20A0, 0x20CF},   /* 货币符号 */
        {0x2100, 0x214F},   /* 字母式符号 */
        {0x2460, 0x24FF},   /* 圈号 ①②③ */
        {0x2500, 0x257F},   /* 制表符 */
        {0x25A0, 0x25FF},   /* 几何符号 ■▲● */
        {0x2600, 0x26FF},   /* 杂项符号 */
        {0x3000, 0x303F},   /* 中文标点 ， 。 「 」 */
        {0x3040, 0x309F},   /* 平假名 */
        {0x30A0, 0x30FF},   /* 片假名 */
        {0x3100, 0x312F},
        {0x3200, 0x32FF},
        {0x4E00, 0x9FFF},   /* CJK 统一表意文字（基本汉字） */
        {0xFF00, 0xFFEF},   /* 半角全角形式 */
    };
    int n = sizeof(ranges) / sizeof(ranges[0]);
    int total = 0;
    for (int i = 0; i < n; i++) total += ranges[i][1] - ranges[i][0] + 1;
    int* arr = (int*)malloc(sizeof(int) * total);
    if (!arr) { *out_total = 0; return NULL; }
    int k = 0;
    for (int i = 0; i < n; i++)
        for (int c = ranges[i][0]; c <= ranges[i][1]; c++) arr[k++] = c;
    *out_total = total;
    return arr;
}

static void load_chinese_font(void) {
    int total = 0;
    int* all = build_codepoints(&total);
    if (!all) {
        g_font = GetFontDefault();
        g_font_loaded = 0;
        TraceLog(LOG_WARNING, "字体 codepoints 分配失败，回退默认字体");
        return;
    }
    g_font = LoadFontEx("assets/fonts/simhei.ttf", FONT_BASE_SIZE, all, total);
    free(all);
    if (g_font.texture.id != 0 && g_font.glyphCount > 0) {
        SetTextureFilter(g_font.texture, TEXTURE_FILTER_BILINEAR);
        g_font_loaded = 1;
        TraceLog(LOG_INFO, "中文字体加载成功 (%d glyphs)", g_font.glyphCount);
    } else {
        UnloadFont(g_font);
        g_font = GetFontDefault();
        g_font_loaded = 0;
        TraceLog(LOG_WARNING, "字体加载失败，回退默认字体");
    }
}

/* ---------- 通用控件 ---------- */
static int draw_button(Rectangle r, const char* text, int fs, int enabled) {
    Vector2 mp = GetMousePosition();
    int hover = enabled && CheckCollisionPointRec(mp, r);
    Color bg = !enabled ? (Color){70,70,70,255}
              : hover    ? (Color){180,140,60,255}
                         : (Color){120,90,40,255};
    DrawRectangleRec(r, bg);
    DrawRectangleLinesEx(r, 2, BLACK);
    int tw = MeasureCN(text, fs);
    DrawCN(text, (int)r.x + ((int)r.width - tw)/2,
                 (int)r.y + ((int)r.height - fs)/2, fs, RAYWHITE);
    return enabled && hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

static void draw_hp_bar(int x, int y, int w, int h, int hp, int max_hp) {
    DrawRectangle(x, y, w, h, (Color){50,50,50,255});
    if (max_hp > 0) {
        Color c = (hp <= 1) ? RED : (hp <= max_hp/2 ? ORANGE : GREEN);
        DrawRectangle(x, y, w * hp / max_hp, h, c);
    }
    DrawRectangleLines(x, y, w, h, BLACK);
}

/* ---------- 场景绘制（不带 BeginDrawing/EndDrawing） ---------- */
static void draw_scene(GameState* gs, int selected_card, const char* hint) {
    ClearBackground((Color){30,30,40,255});

    /* AI 区 */
    for (int i = 1; i < gs->player_count; i++) {
        Character* ai = &gs->players[i];
        Rectangle r = ai_rect(i);
        DrawRectangleRec(r, (Color){60,60,80,255});
        DrawCN(TextFormat("AI%d  %s", i, hero_name(ai->hero)),
               (int)r.x+12, (int)r.y+10, 22, RAYWHITE);
        DrawCN(TextFormat("人格: %s", person_name(ai->personality)),
               (int)r.x+12, (int)r.y+38, 18, LIGHTGRAY);
        DrawCN(TextFormat("血量 %d/%d", ai->hp, ai->max_hp),
               (int)r.x+12, (int)r.y+62, 18, RAYWHITE);
        draw_hp_bar((int)r.x+12, (int)r.y+86, 180, 12, ai->hp, ai->max_hp);
        DrawCN(TextFormat("手牌 %d 张", ai->hand_count),
               (int)r.x+12, (int)r.y+108, 18, RAYWHITE);
        if (ai->is_lord) DrawCN("[主公]", (int)r.x+220, (int)r.y+10, 18, GOLD);
        if (gs->current_turn == i)
            DrawRectangleLinesEx((Rectangle){r.x-2, r.y-2, r.width+4, r.height+4}, 3, GOLD);
    }

    /* 信息条 */
    DrawCN(TextFormat("牌堆 %d 张   弃牌堆 %d 张", gs->pile_count, gs->discard_count),
           40, 210, 18, LIGHTGRAY);
    DrawCN(TextFormat("当前回合: 玩家%d   阶段: %s",
                      gs->current_turn, phase_name(gs->turn_phase)),
           40, 235, 20, YELLOW);

    /* 阶段指示横幅（准备/摸牌阶段特别提示） */
    if (gs->turn_phase == 0) {
        int tw = MeasureCN("--- 准备阶段 ---", 28);
        DrawRectangle(0, 322, WIN_W, 36, (Color){40,40,60,220});
        DrawCN("--- 准备阶段 ---", (WIN_W - tw)/2, 324, 28, (Color){200,200,255,255});
    } else if (gs->turn_phase == 1) {
        int tw = MeasureCN("--- 摸牌阶段 ---", 28);
        DrawRectangle(0, 322, WIN_W, 36, (Color){40,50,40,220});
        DrawCN("--- 摸牌阶段 ---", (WIN_W - tw)/2, 324, 28, (Color){200,255,200,255});
    }

    /* 玩家区 */
    Character* p = &gs->players[0];
    Rectangle pr = player_area_rect();
    DrawRectangleRec(pr, (Color){50,70,50,255});
    DrawCN(TextFormat("玩家  %s", hero_name(p->hero)), 40, 395, 24, RAYWHITE);
    if (p->is_lord) DrawCN("[主公]", 220, 397, 22, GOLD);
    DrawCN(TextFormat("血量 %d/%d", p->hp, p->max_hp), 40, 430, 20, RAYWHITE);
    draw_hp_bar(40, 458, 220, 14, p->hp, p->max_hp);

    /* 手牌 */
    for (int i = 0; i < p->hand_count; i++) {
        Rectangle r = card_rect(i);
        int is_sel = (i == selected_card);
        if (is_sel) {
            r.y -= 12;  /* 抬起 */
            DrawRectangleRec(r, (Color){255,235,150,255});
        } else {
            DrawRectangleRec(r, RAYWHITE);
        }
        DrawRectangleLinesEx(r, is_sel ? 3 : 2, is_sel ? GOLD : BLACK);
        DrawCN(card_name(p->hand[i].type), (int)r.x+8, (int)r.y+10, 18, BLACK);
        DrawCN(TextFormat("#%d", i), (int)r.x+8, (int)(r.y + r.height - 22), 14, DARKGRAY);
    }
    if (gs->current_turn == 0)
        DrawRectangleLinesEx((Rectangle){pr.x-2, pr.y-2, pr.width+4, pr.height+4}, 3, GOLD);

    /* 提示文字 */
    if (hint && hint[0])
        DrawCN(hint, 280, 350, 20, (Color){255,220,120,255});

    /* 浮动错误提示 */
    draw_floating_hint();

    /* 出牌区：最近 N 次出牌（横排，限时显示） */
    {
        int slot_w = 80, slot_h = 56, gap = 8;
        int total_w = slot_w * RECENT_PLAY_MAX + gap * (RECENT_PLAY_MAX - 1);
        int sx = (WIN_W - total_w) / 2;
        int sy = 270;
        DrawCN("最近出牌", sx, sy - 24, 16, LIGHTGRAY);
        double now = GetTime();
        for (int i = 0; i < RECENT_PLAY_MAX; i++) {
            Rectangle r = {sx + i * (slot_w + gap), sy, slot_w, slot_h};
            DrawRectangleRec(r, (Color){45,45,60,255});
            DrawRectangleLinesEx(r, 1, (Color){90,90,110,255});
            if (i < g_recent_count) {
                RecentPlay* rp = &g_recent[g_recent_count - 1 - i];
                double age = now - rp->shown_at;
                if (age > RECENT_VISIBLE_SEC) continue;
                /* 用透明度表达"渐隐" */
                int a = (int)(255 * (1.0 - age / RECENT_VISIBLE_SEC));
                if (a < 60) a = 60;
                Color cc = {255, 245, 200, (unsigned char)a};
                DrawRectangle((int)r.x+1, (int)r.y+1, slot_w-2, slot_h-2, cc);
                DrawCN(card_name(rp->card.type), (int)r.x+6, (int)r.y+4, 18, BLACK);
                const char* who = (rp->actor == 0) ? "玩家" : TextFormat("AI%d", rp->actor);
                if (rp->target >= 0) {
                    const char* tgt = (rp->target == 0) ? "玩家" : TextFormat("AI%d", rp->target);
                    DrawCN(TextFormat("%s→%s", who, tgt), (int)r.x+6, (int)r.y+30, 14, DARKGRAY);
                } else {
                    DrawCN(who, (int)r.x+6, (int)r.y+30, 14, DARKGRAY);
                }
            }
        }
    }

    /* 心理气泡 */
    if (g_psych_msg[0]) {
        if (GetTime() < g_psych_until) {
            Rectangle b = {320, 240, 660, 100};
            DrawRectangleRec(b, (Color){255,255,210,235});
            DrawRectangleLinesEx(b, 2, BLACK);
            DrawCN(TextFormat("[AI心理 表情#%d]", g_psych_emotion),
                   (int)b.x+14, (int)b.y+10, 16, DARKGRAY);
            DrawCN(g_psych_msg, (int)b.x+14, (int)b.y+38, 22, BLACK);
        } else {
            g_psych_msg[0] = '\0';   /* 超时主动清空，避免残留 */
        }
    }

    /* 事件日志 */
    draw_events(gs);
}

/* ============================================================
 *  对外接口
 * ============================================================ */
void ui_init(void) {
    if (g_window_ready) return;
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(WIN_W, WIN_H, "三国杀 - 人格分裂AI");
    SetTargetFPS(60);
    load_chinese_font();
    g_window_ready = 1;
}

void ui_close(void) {
    if (!g_window_ready) return;
    if (g_font_loaded) UnloadFont(g_font);
    CloseWindow();
    g_window_ready = 0;
    g_font_loaded = 0;
}

/* 关窗时统一处理：彻底关闭并退出进程，避免主循环继续调用 raylib */
static void quit_if_window_closed(void) {
    if (g_window_ready && WindowShouldClose()) {
        ui_close();
        exit(0);
    }
}

void ui_draw_game(GameState* gs) {
    if (!g_window_ready) ui_init();
    BeginDrawing();
    draw_scene(gs, -1, NULL);
    EndDrawing();
}

void ui_show_psych_message(const char* message, int emotion_id) {
    if (!message) return;
    strncpy(g_psych_msg, message, sizeof(g_psych_msg)-1);
    g_psych_msg[sizeof(g_psych_msg)-1] = '\0';
    g_psych_emotion = emotion_id;
    g_psych_until = GetTime() + 1.8;
    printf("[AI心理 #%d] %s\n", emotion_id, message);
}

/* ----- 玩家行动：状态机（未选 → 选卡 → 选目标） ----- */
Action ui_get_player_action(GameState* gs) {
    Action act = {1, -1, -1};
    if (!g_window_ready) ui_init();
    Character* p = &gs->players[0];

    int selected = -1;
    const char* hint = "请点击手牌选择，或点击 [结束回合]";

    while (1) {
        quit_if_window_closed();
        Vector2 mp = GetMousePosition();
        int clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

        BeginDrawing();
        draw_scene(gs, selected, hint);
        int end_clicked = draw_button(end_btn_rect(), "结束回合", 20, 1);
        EndDrawing();

        if (end_clicked) {
            act.action_type = 1; act.card_index = -1; act.target = -1;
            return act;
        }
        if (!clicked) continue;

        /* 命中手牌？ */
        int hit_card = -1;
        for (int i = 0; i < p->hand_count; i++) {
            Rectangle r = card_rect(i);
            if (i == selected) r.y -= 12;
            if (CheckCollisionPointRec(mp, r)) { hit_card = i; break; }
        }
        if (hit_card >= 0) {
            CardType ct = p->hand[hit_card].type;
            if (ct == CARD_SHA) {
                /* 检查是否还有出杀次数 */
                int is_zhangfei = (p->hero == HERO_ZHANG_FEI);
                if (!is_zhangfei && gs->sha_used_this_turn >= 1) {
                    show_floating_hint("本回合已使用过【杀】", RED, 1.5f);
                    selected = -1;
                    continue;
                }
            }
            if (ct == CARD_SHAN) {
                int is_zhaoyun = (p->hero == HERO_ZHAO_YUN);
                if (!is_zhaoyun) {
                    hint = "【闪】只能在被【杀】时使用，不可主动出";
                    selected = -1;
                    continue;
                }
                int has_target = 0;
                for (int t = 1; t < gs->player_count; t++)
                    if (gs->players[t].hp > 0 &&
                        !skill_empty_city_blocks_sha(gs, t)) has_target = 1;
                if (!has_target) {
                    hint = "没有可攻击的目标";
                    selected = -1;
                    continue;
                }
            }
            if (selected == hit_card) selected = -1;
            else {
                selected = hit_card;
                if (ct == CARD_TAO || ct == CARD_WU_ZHONG)
                    hint = "请点击自己（玩家区）确认使用";
                else if (ct == CARD_SHA || ct == CARD_GUO_CAI)
                    hint = "请点击目标 AI";
                else
                    hint = "请点击目标";
            }
            continue;
        }

        /* 已选卡 → 命中目标 */
        if (selected >= 0) {
            CardType ct = p->hand[selected].type;
            int hit_target = -1;
            if (CheckCollisionPointRec(mp, player_area_rect())) hit_target = 0;
            else for (int i = 1; i < gs->player_count; i++)
                if (CheckCollisionPointRec(mp, ai_rect(i))) { hit_target = i; break; }
            if (hit_target < 0) continue;

            /* 目标合法性 —— 与三国杀规则一致的白名单 */
            int ok = 0;
            switch (ct) {
                case CARD_TAO:
                case CARD_WU_ZHONG:
                    ok = (hit_target == 0);
                    if (!ok) hint = "该牌只能对自己使用";
                    break;
                case CARD_SHA:
                    ok = (hit_target != 0);
                    if (!ok) {
                        hint = "该牌不能对自己使用";
                    } else if (skill_empty_city_blocks_sha(gs, hit_target)) {
                        show_floating_hint("【空城】目标无手牌，不能成为【杀】的目标", RED, 1.5f);
                        ok = 0;
                    }
                    break;
                case CARD_GUO_CAI:
                    ok = (hit_target != 0);
                    if (!ok) hint = "该牌不能对自己使用";
                    break;
                case CARD_SHAN:
                    ok = (p->hero == HERO_ZHAO_YUN && hit_target != 0);
                    if (!ok) hint = "闪只能在本回合作为【杀】对他人使用";
                    break;
                default:
                    ok = 1;
            }
            if (!ok) continue;

            act.action_type = 0; act.card_index = selected; act.target = hit_target;
            act.use_longdan_sha = 0;
            act.use_longdan_shan = 0;
            if (p->hero == HERO_ZHAO_YUN && ct == CARD_SHAN && hit_target != 0)
                act.use_longdan_sha = 1;
            return act;
        }
    }
}

void ui_show_gameover(int winner) {
    if (!g_window_ready) ui_init();
    double until = GetTime() + 3.5;
    while (GetTime() < until) {
        if (WindowShouldClose()) break;   /* 用户主动关窗，提前结束 */
        BeginDrawing();
        ClearBackground(BLACK);
        const char* msg = (winner == 0) ? "玩家胜利！" : "AI胜利！";
        int fs = 60;
        int tw = MeasureCN(msg, fs);
        DrawCN(msg, (WIN_W-tw)/2, WIN_H/2 - fs/2, fs, (winner == 0 ? GOLD : RED));
        DrawCN("游戏结束", WIN_W/2 - MeasureCN("游戏结束",30)/2,
               WIN_H/2 + 60, 30, GRAY);
        EndDrawing();
    }
}

/* ----- 闪应答弹窗 ----- */
int ui_get_shan_response(GameState* gs) {
    if (!g_window_ready) ui_init();
    Character* p = &gs->players[0];

    int has_shan = 0;
    int is_zhaoyun_p = (p->hero == HERO_ZHAO_YUN);
    for (int i = 0; i < p->hand_count; i++) {
        CardType t = p->hand[i].type;
        if (t == CARD_SHAN || (is_zhaoyun_p && t == CARD_SHA)) { has_shan = 1; break; }
    }
    if (!has_shan) {
        double until = GetTime() + 0.8;
        while (GetTime() < until) {
            quit_if_window_closed();
            BeginDrawing();
            draw_scene(gs, -1, NULL);
            DrawRectangle(0, 0, WIN_W, WIN_H, (Color){0,0,0,140});
            const char* m = "你被【杀】了，但没有【闪】，承受伤害";
            int fs = 26, tw = MeasureCN(m, fs);
            DrawCN(m, (WIN_W - tw)/2, WIN_H/2 - fs/2, fs, RAYWHITE);
            EndDrawing();
        }
        return -1;
    }

    while (1) {
        quit_if_window_closed();
        Vector2 mp = GetMousePosition();
        int clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

        BeginDrawing();
        draw_scene(gs, -1, NULL);
        DrawRectangle(0, 0, WIN_W, WIN_H, (Color){0,0,0,140});
        const char* m = is_zhaoyun_p
            ? "你被【杀】了：点击【闪】或【杀】抵消，或点 [承受伤害]"
            : "你被【杀】了：点击一张【闪】抵消，或点 [承受伤害]";
        int fs = 24, tw = MeasureCN(m, fs);
        DrawCN(m, (WIN_W - tw)/2, 320, fs, (Color){255,220,120,255});
        Rectangle giveup = {END_BTN_X-160, END_BTN_Y, 150, END_BTN_H};
        int gv = draw_button(giveup, "承受伤害", 20, 1);
        EndDrawing();

        if (gv) return -1;
        if (clicked) {
            for (int i = 0; i < p->hand_count; i++) {
                CardType t = p->hand[i].type;
                if (!CheckCollisionPointRec(mp, card_rect(i))) continue;
                if (t == CARD_SHAN || (is_zhaoyun_p && t == CARD_SHA)) return i;
            }
        }
    }
}

/* ----- 弃牌选择弹窗 ----- */
int ui_get_discard_choice(GameState* gs, int remaining) {
    if (!g_window_ready) ui_init();
    Character* p = &gs->players[0];

    while (1) {
        quit_if_window_closed();
        Vector2 mp = GetMousePosition();
        int clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

        BeginDrawing();
        draw_scene(gs, -1, NULL);
        DrawRectangle(0, 0, WIN_W, WIN_H, (Color){0,0,0,140});
        char buf[96];
        snprintf(buf, sizeof(buf), "弃牌阶段：还需弃 %d 张，点击一张手牌", remaining);
        int fs = 24, tw = MeasureCN(buf, fs);
        DrawCN(buf, (WIN_W - tw)/2, 320, fs, (Color){255,220,120,255});
        EndDrawing();

        if (clicked) {
            for (int i = 0; i < p->hand_count; i++) {
                if (CheckCollisionPointRec(mp, card_rect(i))) return i;
            }
        }
    }
}

/* ============================================================
 *  M6 主菜单：模式 → 武将 → 阵营 三屏状态机
 * ============================================================ */

#include <time.h>

/* 已选择 + hover 状态下的按钮卡片 */
static int draw_choice_card(Rectangle r, const char* title, const char* desc,
                            int selected, int hover) {
    Color bg;
    if (selected)      bg = (Color){200,160,70,255};
    else if (hover)    bg = (Color){90,90,120,255};
    else               bg = (Color){60,60,80,255};
    DrawRectangleRec(r, bg);
    DrawRectangleLinesEx(r, selected ? 3 : 2, selected ? GOLD : BLACK);
    DrawCN(title, (int)r.x + 16, (int)r.y + 14, 26, RAYWHITE);
    if (desc) DrawCN(desc, (int)r.x + 16, (int)r.y + 50, 16, LIGHTGRAY);
    return hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

static void random_fill_ai(MenuChoice* mc, int avoid_player_hero, int avoid_camp_hero) {
    /* 给 AI 分配武将（避免与玩家相同；主公局尽量不让两个 AI 武将完全雷同） */
    HeroType pool[3] = {HERO_ZHANG_FEI, HERO_ZHAO_YUN, HERO_ZHU_GE_LIANG};
    /* 先从池里挑可用的（不等于玩家武将） */
    HeroType avail[3]; int n = 0;
    for (int i = 0; i < 3; i++)
        if (pool[i] != avoid_player_hero) avail[n++] = pool[i];
    /* 随机洗一遍 */
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        HeroType t = avail[i]; avail[i] = avail[j]; avail[j] = t;
    }
    mc->ai_hero[0] = avail[0];
    if (mc->ai_count > 1) {
        mc->ai_hero[1] = (n >= 2) ? avail[1] : avail[0];
        (void)avoid_camp_hero;
    }
    /* 人格随机 */
    for (int i = 0; i < mc->ai_count; i++) {
        mc->ai_person[i] = (Personality)(rand() % 3);
    }
}

int ui_show_menu(MenuChoice* out) {
    printf("[MENU DEBUG] Entering ui_show_menu\n");
    fflush(stdout);
    
    if (!g_window_ready) ui_init();
    printf("[MENU DEBUG] UI initialized, g_window_ready=%d\n", g_window_ready);
    fflush(stdout);
    
    if (!out) {
        printf("[MENU DEBUG] out is NULL, returning 0\n");
        fflush(stdout);
        return 0;
    }
    srand((unsigned)time(NULL));

    int step = 0;                  /* 0:模式 1:阵营(主公局) 2:武将 3:确认 */
    int sel_mode = -1;             /* 0=单挑, 1=主公局 */
    int sel_camp = -1;             /* 0=主公, 1=反贼（仅主公局有效） */
    int sel_hero = -1;             /* 0/1/2 */

    printf("[MENU DEBUG] Starting menu loop\n");
    fflush(stdout);
    
    while (1) {
        printf("[MENU DEBUG] Loop iteration, step=%d\n", step);
        fflush(stdout);
        
        quit_if_window_closed();
        Vector2 mp = GetMousePosition();

        BeginDrawing();
        ClearBackground((Color){25,25,35,255});

        /* 标题 */
        const char* title = "三国杀 · 人格分裂AI";
        int tw = MeasureCN(title, 40);
        DrawCN(title, (WIN_W - tw)/2, 40, 40, GOLD);

        /* 步骤指示 */
        const char* steps[] = {"① 选择模式", "② 选择阵营", "③ 选择武将", "④ 确认开始"};
        int nsteps = 4;
        for (int i = 0; i < nsteps; i++) {
            Color c = (i == step) ? YELLOW : (i < step ? GREEN : GRAY);
            DrawCN(steps[i], 80 + i * 220, 110, 20, c);
        }

        if (step == 0) {
            /* 模式 */
            Rectangle r1 = {WIN_W/2 - 380, 200, 360, 140};
            Rectangle r2 = {WIN_W/2 + 20,  200, 360, 140};
            int h1 = CheckCollisionPointRec(mp, r1);
            int h2 = CheckCollisionPointRec(mp, r2);
            int c1 = draw_choice_card(r1, "单挑模式", "玩家 vs 1 个 AI，公平对战", sel_mode==0, h1);
            int c2 = draw_choice_card(r2, "主公局",   "主公 vs 2 反贼，主公增益", sel_mode==1, h2);
            if (c1) {
                sel_mode = 0;
                printf("[MENU DEBUG] Selected mode: 单挑\n");
                fflush(stdout);
            }
            if (c2) {
                sel_mode = 1;
                printf("[MENU DEBUG] Selected mode: 主公局\n");
                fflush(stdout);
            }

            Rectangle next = {WIN_W/2 - 80, 380, 160, 50};
            int nx = draw_button(next, "下一步", 22, sel_mode >= 0);
            if (nx) {
                printf("[MENU DEBUG] Next button clicked, step=%d, sel_mode=%d\n", step, sel_mode);
                fflush(stdout);
                if (sel_mode == 0) { sel_camp = 0; step = 2; }   /* 单挑跳过阵营 */
                else step = 1;
            }
        }
        else if (step == 1) {
            /* 阵营 */
            Rectangle r1 = {WIN_W/2 - 380, 200, 360, 140};
            Rectangle r2 = {WIN_W/2 + 20,  200, 360, 140};
            int h1 = CheckCollisionPointRec(mp, r1);
            int h2 = CheckCollisionPointRec(mp, r2);
            int c1 = draw_choice_card(r1, "主公",  "血量上限 +1，摸牌阶段摸 3 张", sel_camp==0, h1);
            int c2 = draw_choice_card(r2, "反贼",  "与另一 AI 反贼共同对抗 AI 主公", sel_camp==1, h2);
            if (c1) {
                sel_camp = 0;
                printf("[MENU DEBUG] Selected camp: 主公\n");
                fflush(stdout);
            }
            if (c2) {
                sel_camp = 1;
                printf("[MENU DEBUG] Selected camp: 反贼\n");
                fflush(stdout);
            }

            Rectangle back = {WIN_W/2 - 200, 380, 130, 46};
            Rectangle next = {WIN_W/2 +  60, 380, 130, 46};
            if (draw_button(back, "上一步", 20, 1)) step = 0;
            if (draw_button(next, "下一步", 20, sel_camp >= 0)) {
                printf("[MENU DEBUG] Next button clicked, step=%d, sel_camp=%d\n", step, sel_camp);
                fflush(stdout);
                step = 2;
            }
        }
        else if (step == 2) {
            /* 武将 */
            const char* names[] = {"张飞", "赵云", "诸葛亮"};
            const char* descs[] = {
                "咆哮：使用【杀】无次数限制",
                "龙胆：杀闪可互相转化",
                "观星 + 空城：调整牌堆顶；无手牌时不能成为杀的目标"
            };
            for (int i = 0; i < 3; i++) {
                Rectangle r = {80 + i * 290, 200, 260, 160};
                int h = CheckCollisionPointRec(mp, r);
                int c = draw_choice_card(r, names[i], descs[i], sel_hero==i, h);
                if (c) {
                    sel_hero = i;
                    printf("[MENU DEBUG] Selected hero: %d (%s)\n", sel_hero, names[i]);
                    fflush(stdout);
                }
            }

            Rectangle back = {WIN_W/2 - 200, 400, 130, 46};
            Rectangle next = {WIN_W/2 +  60, 400, 130, 46};
            int back_to = (sel_mode == 0) ? 0 : 1;
            if (draw_button(back, "上一步", 20, 1)) step = back_to;
            if (draw_button(next, "开始游戏", 22, sel_hero >= 0)) {
                printf("[MENU DEBUG] Start button clicked, step=%d, sel_hero=%d\n", step, sel_hero);
                fflush(stdout);
                step = 3;
            }
        }
        else if (step == 3) {
            /* 填写并返回 */
            printf("[MENU DEBUG] Step 3: mode=%d, hero=%d, is_lord=%d, ai_count=%d\n", 
                   (sel_mode == 1) ? MODE_LORD_VS_REBELS : MODE_SINGLE,
                   sel_hero,
                   (sel_mode == 1 && sel_camp == 0) ? 1 : 0,
                   (sel_mode == 1) ? 2 : 1);
            fflush(stdout);
            
            out->mode           = (sel_mode == 1) ? MODE_LORD_VS_REBELS : MODE_SINGLE;
            out->player_hero    = (HeroType)sel_hero;
            out->player_is_lord = (sel_mode == 1 && sel_camp == 0) ? 1 : 0;
            out->ai_count       = (sel_mode == 1) ? 2 : 1;
            random_fill_ai(out, out->player_hero, out->player_hero);
            EndDrawing();
            printf("[MENU DEBUG] Returning from ui_show_menu\n");
            fflush(stdout);
            return 1;
        }
        EndDrawing();
    }
}

/* ============================================================
 *  打磨：节奏控制 & 出牌区记录
 * ============================================================ */

void ui_pause(GameState* gs, double seconds) {
    if (!g_window_ready) ui_init();
    double until = GetTime() + seconds;
    while (GetTime() < until) {
        quit_if_window_closed();
        BeginDrawing();
        if (gs) draw_scene(gs, -1, NULL);
        else    ClearBackground((Color){25,25,35,255});
        EndDrawing();
    }
}

void ui_show_card_played(int actor_idx, Card card, int target_idx) {
    /* 队列右移：丢掉最老的，新一条放最末尾 */
    if (g_recent_count < RECENT_PLAY_MAX) {
        g_recent[g_recent_count].actor    = actor_idx;
        g_recent[g_recent_count].target   = target_idx;
        g_recent[g_recent_count].card     = card;
        g_recent[g_recent_count].shown_at = GetTime();
        g_recent_count++;
    } else {
        for (int i = 0; i < RECENT_PLAY_MAX - 1; i++) g_recent[i] = g_recent[i+1];
        g_recent[RECENT_PLAY_MAX - 1].actor    = actor_idx;
        g_recent[RECENT_PLAY_MAX - 1].target   = target_idx;
        g_recent[RECENT_PLAY_MAX - 1].card     = card;
        g_recent[RECENT_PLAY_MAX - 1].shown_at = GetTime();
    }
}

/* 事件显示行数 */
#define EVENT_DISPLAY_LINES 4

/* 内部函数：绘制事件日志 */
static void draw_events(GameState* gs) {
    if (!gs) return;

    int count = game_get_event_count(gs);
    if (count == 0) return;

    /* 固定显示最新的4条事件 */
    const int log_y = WIN_H - 195;  /* 调高三行 */
    const int line_height = 25;
    int start = (count > EVENT_DISPLAY_LINES) ? (count - EVENT_DISPLAY_LINES) : 0;
    int display_count = (count > EVENT_DISPLAY_LINES) ? EVENT_DISPLAY_LINES : count;

    for (int i = 0; i < display_count; i++) {
        GameEvent* evt = game_get_event(gs, start + i);
        if (!evt) continue;

        int y = log_y + i * line_height;

        /* 根据事件类型选择颜色 */
        Color color = WHITE;
        switch (evt->type) {
            case EVENT_GAME_START:
                color = GOLD;
                break;
            case EVENT_TURN_START:
            case EVENT_TURN_END:
                color = YELLOW;
                break;
            case EVENT_PHASE_CHANGE:
                color = LIGHTGRAY;
                break;
            case EVENT_DRAW_CARDS:
                color = SKYBLUE;
                break;
            case EVENT_CARD_PLAYED:
                color = RAYWHITE;
                break;
            case EVENT_SHAN_RESPONSE:
                color = (evt->card_type == CARD_SHAN || evt->card_type == CARD_SHA) ? SKYBLUE : GRAY;
                break;
            case EVENT_TAO_SAVE:
                color = GREEN;
                break;
            case EVENT_DISMANTLE:
                color = ORANGE;
                break;
            case EVENT_DMG_TAKEN:
                color = RED;
                break;
            case EVENT_DEATH:
                color = DARKPURPLE;
                break;
            case EVENT_SKILL_USED:
                color = PURPLE;
                break;
            default:
                color = WHITE;
        }

        /* 技能事件：根据技能类型高亮 */
        int font_size = 18;
        if (evt->type == EVENT_SKILL_USED && evt->skill_type != SKILL_NONE) {
            font_size = 22;
            switch (evt->skill_type) {
                case SKILL_LONG_DAN:
                    color = GOLD;
                    break;
                case SKILL_PAO_XIAO:
                    color = RED;
                    break;
                case SKILL_GUAN_XING:
                    color = PURPLE;
                    break;
                case SKILL_KONG_CHENG:
                    color = SKYBLUE;
                    break;
                default:
                    break;
            }
        }

        /* 右侧对齐显示 */
        int text_width = MeasureCN(evt->message, font_size);
        DrawCN(evt->message, WIN_W - text_width - 20, y, font_size, color);
    }
}

void ui_show_events(GameState* gs) {
    /* 事件现在在 ui_draw_game 中绘制，此函数保留为空以保持接口兼容 */
    (void)gs;
}

/* 显示初始手牌界面（简化版） */
void ui_show_initial_hand(GameState* gs) {
    if (!gs) return;
    if (!g_window_ready) ui_init();
    
    /* 获取先手角色名 */
    const char* first = (gs->current_turn == 0) ? "你" : 
                        (gs->current_turn == 1) ? "AI1" : "AI2";
    
    while (!WindowShouldClose()) {
        quit_if_window_closed();
        Vector2 mp = GetMousePosition();
        int clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
        
        BeginDrawing();
        ClearBackground((Color){20,20,30,255});
        
        /* 标题 */
        int title_w = MeasureCN("游戏开始", 48);
        DrawCN("游戏开始", (WIN_W - title_w)/2, 120, 48, GOLD);
        
        /* 提示信息1 */
        int tip1_w = MeasureCN("各角色已获得初始手牌", 24);
        DrawCN("各角色已获得初始手牌", (WIN_W - tip1_w)/2, 260, 24, LIGHTGRAY);
        
        /* 提示信息2 */
        char tip2[64];
        snprintf(tip2, sizeof(tip2), "由 %s 开始回合", first);
        int tip2_w = MeasureCN(tip2, 28);
        DrawCN(tip2, (WIN_W - tip2_w)/2, 320, 28, RAYWHITE);
        
        /* 确认按钮 */
        Rectangle btn = {WIN_W/2 - 100, WIN_H - 150, 200, 55};
        int hover = CheckCollisionPointRec(mp, btn);
        Color btn_color = hover ? (Color){180,130,50,255} : (Color){100,80,50,255};
        DrawRectangleRec(btn, btn_color);
        DrawRectangleLinesEx(btn, 3, hover ? GOLD : (Color){200,160,80,255});
        
        int btn_text_w = MeasureCN("开始游戏", 28);
        DrawCN("开始游戏", (int)btn.x + ((int)btn.width - btn_text_w)/2,
               (int)btn.y + 14, 28, RAYWHITE);
        
        EndDrawing();
        
        /* 点击确认或等待1.5秒后自动进入 */
        static double start_time = 0;
        if (start_time == 0) start_time = GetTime();
        
        if ((clicked && CheckCollisionPointRec(mp, btn)) || 
            (GetTime() - start_time > 1.5)) {
            break;
        }
    }
}

/* 获取玩家是否使用桃救人的选择（返回桃的索引，-1表示不救） */
int ui_get_tao_save_choice(GameState* gs, int dying_idx) {
    if (!gs || dying_idx < 0 || dying_idx >= gs->player_count) return -1;
    if (!g_window_ready) ui_init();
    
    Character* p = &gs->players[0];
    const char* dying_name = (dying_idx == 0) ? "你" : 
                            (dying_idx == 1) ? "AI1" : "AI2";
    
    /* 检查是否有桃 */
    int has_tao = 0;
    for (int i = 0; i < p->hand_count; i++) {
        if (p->hand[i].type == CARD_TAO) {
            has_tao = 1;
            break;
        }
    }
    
    /* 如果没有桃，直接返回不救 */
    if (!has_tao) {
        double until = GetTime() + 0.8;
        while (GetTime() < until) {
            quit_if_window_closed();
            BeginDrawing();
            draw_scene(gs, -1, NULL);
            DrawRectangle(0, 0, WIN_W, WIN_H, (Color){0,0,0,140});
            char m[128];
            snprintf(m, sizeof(m), "%s 濒死，但你没有【桃】可以救他", dying_name);
            int fs = 26, tw = MeasureCN(m, fs);
            DrawCN(m, (WIN_W - tw)/2, WIN_H/2 - fs/2, fs, RAYWHITE);
            EndDrawing();
        }
        return -1;
    }
    
    /* 显示选择界面 */
    while (1) {
        quit_if_window_closed();
        Vector2 mp = GetMousePosition();
        int clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
        
        BeginDrawing();
        draw_scene(gs, -1, NULL);
        DrawRectangle(0, 0, WIN_W, WIN_H, (Color){0,0,0,140});
        
        /* 提示文字 */
        char m[128];
        snprintf(m, sizeof(m), "%s 处于濒死状态：点击一张【桃】救他，或点 [不救]", dying_name);
        int fs = 24, tw = MeasureCN(m, fs);
        DrawCN(m, (WIN_W - tw)/2, 320, fs, (Color){255,220,120,255});
        
        /* 不救按钮 */
        Rectangle giveup = {END_BTN_X-160, END_BTN_Y, 150, END_BTN_H};
        int gv = draw_button(giveup, "不救", 20, 1);
        
        EndDrawing();
        
        /* 点击不救按钮 */
        if (gv) return -1;
        
        /* 点击手牌 */
        if (clicked) {
            for (int i = 0; i < p->hand_count; i++) {
                if (!CheckCollisionPointRec(mp, card_rect(i))) continue;
                if (p->hand[i].type == CARD_TAO) {
                    return i;  // 返回选中的桃的索引
                }
            }
        }
    }
}

void ui_get_star_choice(GameState* gs) {
    if (!gs || !gs->need_star_choice) return;
    if (!g_window_ready) ui_init();

    int selected = -1;
    const int count = gs->star_watch_count;

    while (!WindowShouldClose()) {
        quit_if_window_closed();
        Vector2 mp = GetMousePosition();
        int clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

        BeginDrawing();
        ClearBackground((Color){25,25,35,255});

        int title_w = MeasureCN("观  星", 36);
        DrawCN("观  星", (WIN_W - title_w)/2, 50, 36, GOLD);
        int tw = MeasureCN("点击两张牌交换位置，排列完成后点确认", 20);
        DrawCN("点击两张牌交换位置，排列完成后点确认",
               (WIN_W - tw)/2, 100, 20, LIGHTGRAY);

        int card_w = 140, card_h = 110, gap = 20;
        int total_w = count * card_w + (count - 1) * gap;
        int start_x = (WIN_W - total_w) / 2;
        int y = 180;

        for (int i = 0; i < count; i++) {
            Rectangle r = {start_x + i * (card_w + gap), y, card_w, card_h};
            int ci = gs->star_current_slots[i];
            Color bg = (i == selected) ? (Color){180,120,40,255}
                     : (CheckCollisionPointRec(mp, r) ? (Color){100,80,60,255}
                        : (Color){60,50,40,255});
            DrawRectangleRec(r, bg);
            DrawRectangleLinesEx(r, 2, (Color){200,160,80,255});

            const char* cname = card_name(gs->star_watch_cards[ci].type);
            int nw = MeasureCN(cname, 24);
            DrawCN(cname, (int)r.x + ((int)r.width - nw)/2,
                          (int)r.y + 20, 24, RAYWHITE);
            char buf[16];
            sprintf(buf, "%d", i + 1);
            DrawCN(buf, (int)r.x + 8, (int)r.y + 60, 18, LIGHTGRAY);

            if (i > 0) {
                DrawCN("↑↓", (int)r.x - gap, (int)r.y + (int)r.height/2 - 10,
                       20, (Color){150,150,150,255});
            }

            if (clicked && CheckCollisionPointRec(mp, r)) {
                if (selected == -1) {
                    selected = i;
                } else if (selected != i) {
                    game_swap_star_slots(gs, selected, i);
                    selected = -1;
                } else {
                    selected = -1;
                }
            }
        }

        int btn_y = y + card_h + 30;
        Rectangle confirm_btn = {WIN_W/2 - 80, btn_y, 160, 45};
        if (draw_button(confirm_btn, "确认排列", 22, 1)) {
            EndDrawing();
            break;
        }

        EndDrawing();
    }
}
