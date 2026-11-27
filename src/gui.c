#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "gui.h"
#include "ui_color.h"
#include "keyword.h"
#include "detect.h"

// main.c에서 불러온 폰트 공유
static Font gui_font;

void gui_init(Font font) {
    gui_font = font;
}

// 일반 텍스트 출력
static void text(const char* str, float x, float y,
                 float size, Color color) {

    DrawTextEx(
        gui_font, str,
        (Vector2){x, y},
        size, UI_FONT_SPACING, color
    );
}

// 가운데 정렬 텍스트
static void center_text(const char* str, float y,
                        float size, Color color) {

    Vector2 t = MeasureTextEx(
        gui_font, str, size, UI_FONT_SPACING
    );

    text(str, (UI_WIDTH - t.x) / 2, y, size, color);
}

// 상단 헤더
static void header(const char* title) {

    DrawRectangleRounded(
        (Rectangle){24, 24, 752, 88},
        0.25f, 10, UI_PRIMARY
    );

    center_text(
        title, 34,
        UI_FONT_TITLE, UI_WHITE
    );
}

// 버튼 출력 및 클릭 확인
static int button(Rectangle r, const char* str) {

    int hover = CheckCollisionPointRec(
        GetMousePosition(), r
    );

    DrawRectangleRounded(
        r, UI_ROUNDNESS, 10,
        hover ? UI_HOVER : UI_SURFACE
    );

    Vector2 t = MeasureTextEx(
        gui_font, str,
        UI_FONT_BODY, UI_FONT_SPACING
    );

    text(
        str,
        r.x + (r.width - t.x) / 2,
        r.y + (r.height - t.y) / 2,
        UI_FONT_BODY, UI_TEXT
    );

    return hover &&
           IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

// 정보 카드 배경
static void card(Rectangle r) {

    DrawRectangleRounded(
        r, UI_ROUNDNESS, 10, UI_SURFACE
    );
}

// 초 단위 시간을 HH:MM:SS 형식으로 변환
static void format_time(int sec, char* buf, size_t size) {

    snprintf(
        buf, size, "%02d:%02d:%02d",
        sec / 3600,
        sec / 60 % 60,
        sec % 60
    );
}

// 진행률 표시
static void progress_bar(Rectangle r, float progress) {

    if (progress < 0) progress = 0;
    if (progress > 1) progress = 1;

    DrawRectangleRounded(
        r, 0.5f, 10, UI_HOVER
    );

    r.width *= progress;

    if (r.width > 0) {
        DrawRectangleRounded(
            r, 0.5f, 10, UI_PRIMARY
        );
    }
}

// 학습 계획 초기화
void setup_init(SetupState* s) {
    memset(s, 0, sizeof(*s));
    s->active_field = 1;
}

// 입력창 위치
static const Rectangle subject_rect = {36, 165, 728, 50};
static const Rectangle goal_rect = {36, 275, 728, 50};
static const Rectangle start_rect = {410, 397, 354, 52};
static const Rectangle back_rect = {36, 397, 354, 52};

// 입력창 출력
static void input_box(Rectangle r, const char* str,
                      int active) {

    card(r);

    DrawRectangleLinesEx(
        r, 2,
        active ? UI_PRIMARY : UI_BORDER
    );

    text(str, r.x + 15, r.y + 10,
         UI_FONT_BODY, UI_TEXT);
}

// 학습 계획 입력 처리
int setup_update(SetupState* s, Session* rec) {

    // 입력창 선택
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mouse = GetMousePosition();

        if (CheckCollisionPointRec(mouse, subject_rect))
            s->active_field = 1;

        else if (CheckCollisionPointRec(mouse, goal_rect))
            s->active_field = 2;
    }

    // Tab으로 입력창 이동
    if (IsKeyPressed(KEY_TAB))
        s->active_field = s->active_field == 1 ? 2 : 1;

    char* buf = s->active_field == 1 ?
                s->subject : s->goal;

    // 문자 입력
    int ch = GetCharPressed();

    while (ch > 0) {

        int len = strlen(buf);

        if (s->active_field == 1 &&
            ch >= 32 && ch <= 0xFFFF &&
            GetCodepointCount(buf) < MAX_SUBJECT_LEN) {

            int bytes;
            const char* utf8 = CodepointToUTF8(ch, &bytes);

            if (len + bytes < sizeof(s->subject)) {
                memcpy(buf + len, utf8, bytes);
                buf[len + bytes] = '\0';
            }
        }

        else if (s->active_field == 2 &&
                 ch >= '0' && ch <= '9' &&
                 len < 3) {

            buf[len] = (char)ch;
            buf[len + 1] = '\0';
        }

        ch = GetCharPressed();
    }

    // Backspace
    if (IsKeyPressed(KEY_BACKSPACE)) {

        int len = strlen(buf);

        if (len > 0) {
            len--;

            while (len > 0 &&
                  ((unsigned char)buf[len] & 0xC0) == 0x80)
                len--;

            buf[len] = '\0';
        }
    }

    // 메인으로 돌아가기
    if (IsKeyPressed(KEY_ESCAPE) ||
        (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
         CheckCollisionPointRec(GetMousePosition(), back_rect)))
        return -1;

    // 공부 시작
    if (IsKeyPressed(KEY_ENTER) ||
        (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
         CheckCollisionPointRec(GetMousePosition(), start_rect))) {

        int goal = atoi(s->goal);

        if (s->subject[0] == '\0' ||
            goal < 1 || goal > 600)
            return 0;

        memset(rec, 0, sizeof(*rec));

        strcpy(rec->user_name, "User");
        strcpy(rec->subject, s->subject);

        rec->goal_min = goal;

        // 날짜 저장
        time_t now = time(NULL);
        struct tm* t = localtime(&now);

        if (t != NULL)
            strftime(rec->date, sizeof(rec->date),
                     "%Y-%m-%d", t);

        return 1;
    }

    return 0;
}

// 학습 계획 화면 출력
void setup_draw(const SetupState* s) {

    header("학습 계획 설정");

    text("공부 제목", 36, 125,
         UI_FONT_BODY, UI_TEXT);

    input_box(subject_rect, s->subject,
              s->active_field == 1);

    text("목표 공부 시간 (분)", 36, 235,
         UI_FONT_BODY, UI_TEXT);

    input_box(goal_rect, s->goal,
              s->active_field == 2);

    text("1~600분 사이로 입력하세요.",
         36, 340, 20, UI_MUTED);

    button(back_rect, "메인으로");
    button(start_rect, "공부 시작");
}

// 학습 진행 화면
void draw_study_screen(
    const char* subject,
    int goal_min,
    int study_sec,
    int distract_sec,
    int distract_count
) {
    char buf[128];
    int elapsed = study_sec + distract_sec;

    float progress = 0;

    if (goal_min > 0)
        progress = (float)study_sec / (goal_min * 60);

    if (progress > 1)
        progress = 1;

    // 상단 제목
    header("학습 진행");

    // 학습 과목
    center_text(subject, 125,
                UI_FONT_SUBTITLE, UI_TEXT);

    // 순수 공부 시간
    center_text("순수 공부 시간", 175,
                UI_FONT_SMALL, UI_MUTED);

    format_time(study_sec, buf, sizeof(buf));

    center_text(buf, 205,
                UI_FONT_TIMER, UI_PRIMARY);

    // 목표 시간 및 달성률
    snprintf(buf, sizeof(buf),
             "목표 %d분 / 달성률 %.0f%%",
             goal_min, progress * 100);

    center_text(buf, 290,
                UI_FONT_SMALL, UI_TEXT);

    // 진행률 막대
    progress_bar(
        (Rectangle){36, 325, 728, 12},
        progress
    );

    // 정보 카드
    card((Rectangle){36, 350, 232, 65});
    card((Rectangle){284, 350, 232, 65});
    card((Rectangle){532, 350, 232, 65});

    // 총 경과 시간
    text("총 경과 시간", 50, 355, 18, UI_MUTED);

    format_time(elapsed, buf, sizeof(buf));
    text(buf, 50, 377, 26, UI_TEXT);

    // 학습 이탈 시간
    text("학습 이탈 시간", 298, 355, 18, UI_MUTED);

    format_time(distract_sec, buf, sizeof(buf));
    text(buf, 298, 377, 26, UI_DISTRACT);

    // 학습 이탈 횟수
    text("학습 이탈 횟수", 546, 355, 18, UI_MUTED);

    snprintf(buf, sizeof(buf), "%d회", distract_count);
    text(buf, 546, 377, 26, UI_TEXT);
}


// 학습 결과 화면
void draw_result_screen(
    const char* subject,
    int goal_min,
    int study_sec,
    int distract_sec,
    int distract_count,
    int earn,
    int penalty,
    int bonus,
    int total_points
) {
    char buf[128];

    int elapsed = study_sec + distract_sec;
    int net_points = earn - penalty + bonus;

    double focus_rate = 0;

    if (elapsed > 0)
        focus_rate = study_sec * 100.0 / elapsed;

    int achieved = study_sec >= goal_min * 60;

    // 상단 제목
    header("학습 결과");

    // 학습 제목 및 목표 달성 여부
    text(subject, 36, 125, 26, UI_TEXT);

    text(
        achieved ? "목표 달성!" : "목표 미달성",
        550, 125, 26,
        achieved ? UI_FOCUS : UI_DISTRACT
    );

    // 학습 결과 카드
    card((Rectangle){36, 165, 354, 240});

    text("STUDY REPORT", 52, 177, 24, UI_PRIMARY);

    format_time(elapsed, buf, sizeof(buf));
    text("총 경과 시간", 52, 215, 20, UI_MUTED);
    text(buf, 245, 215, 20, UI_TEXT);

    format_time(study_sec, buf, sizeof(buf));
    text("순수 공부 시간", 52, 250, 20, UI_MUTED);
    text(buf, 245, 250, 20, UI_FOCUS);

    format_time(distract_sec, buf, sizeof(buf));
    text("학습 이탈 시간", 52, 285, 20, UI_MUTED);
    text(buf, 245, 285, 20, UI_DISTRACT);

    snprintf(buf, sizeof(buf), "%d회", distract_count);
    text("학습 이탈 횟수", 52, 320, 20, UI_MUTED);
    text(buf, 245, 320, 20, UI_TEXT);

    snprintf(buf, sizeof(buf), "%.1f%%", focus_rate);
    text("집중률", 52, 355, 20, UI_MUTED);
    text(buf, 245, 355, 24, UI_FOCUS);

    // 포인트 결과 카드
    card((Rectangle){410, 165, 354, 240});

    text("POINT REPORT", 426, 177, 24, UI_PRIMARY);

    snprintf(buf, sizeof(buf), "+%d P", earn);
    text("획득 포인트", 426, 215, 20, UI_MUTED);
    text(buf, 615, 215, 20, UI_FOCUS);

    snprintf(buf, sizeof(buf), "-%d P", penalty);
    text("감점 포인트", 426, 250, 20, UI_MUTED);
    text(buf, 615, 250, 20, UI_DISTRACT);

    snprintf(buf, sizeof(buf), "+%d P", bonus);
    text("보너스 포인트", 426, 285, 20, UI_MUTED);
    text(buf, 615, 285, 20, UI_WARNING);

    DrawLine(426, 315, 748, 315, UI_BORDER);

    snprintf(buf, sizeof(buf), "%+d P", net_points);
    text("총 획득 포인트", 426, 325, 20, UI_TEXT);
    text(buf, 615, 325, 24, UI_PRIMARY);

    if (total_points >= 0)
        snprintf(buf, sizeof(buf), "%d P", total_points);
    else
        snprintf(buf, sizeof(buf), "저장 실패");

    text("누적 포인트", 426, 365, 20, UI_MUTED);
    text(buf, 615, 365, 20, UI_TEXT);
}

// 학습 이탈 화면
void draw_warning_screen(int keyword_index) {

    header("학습 이탈 감지");

    card((Rectangle){36, 130, 728, 275});

    // 경고 아이콘
    DrawCircle(400, 200, 45, UI_DISTRACT);

    center_text("!", 165, 65, UI_WHITE);

    // 경고 메시지
    center_text("학습 이탈이 감지되었습니다!",
                260, 32, UI_DISTRACT);

    // 감지된 키워드 출력
    const char* word = keyword_at(keyword_index);

    if (word != NULL) {
        char buf[128];

        snprintf(buf, sizeof(buf),
                 "감지 키워드 : %s", word);

        center_text(buf, 310, 25, UI_TEXT);
    }

    center_text("공부에 다시 집중하세요.",
                355, 25, UI_MUTED);
}

// 학습 기록 저장 실패 안내
void draw_save_error(void) {

    text("학습 기록 저장에 실패했습니다.",
         36, 430, 20, UI_DISTRACT);
}



