#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

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

    Rectangle r = {24, 24, 752, 88};

    DrawRectangleRounded(
        r, 0.25f, 10, UI_PRIMARY
    );

    Vector2 t = MeasureTextEx(
        gui_font, title,
        UI_FONT_TITLE, UI_FONT_SPACING
    );

    center_text(
        title,
        r.y + (r.height - t.y) / 2,
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
// 이미 등록된 과목인지 확인
static int find_subject(const SetupState* s, const char* name) {

    for (int i = 0; i < s->subject_count; i++) {
        if (strcmp(s->subjects[i], name) == 0)
            return i;
    }

    return -1;
}

// 기존 학습 기록에서 과목 추출
static void import_subjects(SetupState* s) {

    int count = loadSessions(NULL, 0);

    if (count <= 0)
        return;

    Session* list = malloc(sizeof(Session) * count);

    if (list == NULL)
        return;

    int loaded = loadSessions(list, count);
    int changed = 0;

    if (loaded > count)
        loaded = count;

    for (int i = 0; i < loaded; i++) {

        if (s->subject_count >= MAX_SUBJECTS)
            break;

        if (list[i].subject[0] != '\0' &&
            find_subject(s, list[i].subject) == -1) {

            strcpy(
                s->subjects[s->subject_count],
                list[i].subject
            );

            s->subject_count++;
            changed = 1;
        }
    }

    free(list);

    // 추가된 과목 목록 저장
    if (changed) {
        if (!saveSubjects(s->subjects, s->subject_count))
            strcpy(s->error, "과목 목록 저장 실패");
    }
}

// 새로운 과목 추가 및 저장
static int add_subject(SetupState* s) {

    // 빈 과목명 검사
    if (s->new_subject[0] == '\0' ||
        strspn(s->new_subject, " \t") == strlen(s->new_subject)) {

        strcpy(s->error, "과목명을 입력하세요.");
        return 0;
    }

    // 중복 과목 검사
    if (find_subject(s, s->new_subject) != -1) {
        strcpy(s->error, "이미 등록된 과목입니다.");
        return 0;
    }

    // 최대 과목 개수 검사
    if (s->subject_count >= MAX_SUBJECTS) {
        strcpy(s->error, "과목 목록이 가득 찼습니다.");
        return 0;
    }

    // 과목 목록에 추가
    strcpy(
        s->subjects[s->subject_count],
        s->new_subject
    );

    s->subject_count++;

    // 파일 저장
    if (!saveSubjects(s->subjects, s->subject_count)) {
        s->subject_count--;
        strcpy(s->error, "과목 저장에 실패했습니다.");
        return 0;
    }

    // 새로 추가한 과목 자동 선택
    s->selected_subject = s->subject_count - 1;

    strcpy(s->subject, s->new_subject);

    // 과목 추가창 초기화
    s->new_subject[0] = '\0';
    s->error[0] = '\0';
    s->adding_subject = 0;
    s->active_field = 0;

    return 1;
}

// 학습 계획 초기화
void setup_init(SetupState* s) {
    memset(s, 0, sizeof(*s));

    // 등록된 과목 목록 불러오기
    s->subject_count = loadSubjects(
        s->subjects, MAX_SUBJECTS
    );

    import_subjects(s);

    // 아직 과목을 선택하지 않은 상태
    s->selected_subject = -1;

    // 과목 추가 상태 초기화
    s->adding_subject = 0;

    // 스크롤 위치 초기화
    s->scroll = 0;

    // 입력창 선택 초기화
    s->active_field = 0;

    s->dropdown_open = 0;
    strcpy(s->goal, "60");
}

// 과목 선택
static const Rectangle dropdown_rect = {160, 120, 480, 52};
static const Rectangle subject_list_rect = {160, 172, 480, 210};
static const Rectangle add_rect = {250, 180, 300, 35};

// 목표 시간
static const Rectangle goal_rect = {310, 260, 180, 85};
static const Rectangle minus_rect = {247, 277, 46, 46};
static const Rectangle plus_rect = {507, 277, 46, 46};

// 빠른 시간 선택
static const int preset_minutes[] = {25, 30, 60, 120};

// 화면 이동
static const Rectangle back_rect = {24, 32, 50, 50};
static const Rectangle start_rect = {36, 420, 728, 44};

// 과목 추가창
static const Rectangle new_subject_rect = {150, 195, 500, 50};
static const Rectangle confirm_rect = {410, 275, 240, 50};
static const Rectangle cancel_rect = {150, 275, 240, 50};

#define VISIBLE_SUBJECTS 5
#define SUBJECT_ROW_HEIGHT 42

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

// 문자열 입력 및 삭제 처리
static void edit_input(char* buf, int capacity,
                       int max_chars, int number_only) {

    int ch = GetCharPressed();

    while (ch > 0) {
        int len = strlen(buf);

        if (ch >= 32 && ch <= 0xFFFF &&
            GetCodepointCount(buf) < max_chars &&
            (!number_only || (ch >= '0' && ch <= '9'))) {

            int bytes;
            const char* utf8 = CodepointToUTF8(ch, &bytes);

            if (len + bytes < capacity) {
                memcpy(buf + len, utf8, bytes);
                buf[len + bytes] = '\0';
            }
        }

        ch = GetCharPressed();
    }

    // Backspace 처리
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
}

// 버튼 클릭 검사
static int clicked(Rectangle r) {
    return IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
           CheckCollisionPointRec(GetMousePosition(), r);
}

// 과목명 앞뒤 공백 제거
static void trim_subject(char* str) {

    int start = strspn(str, " \t");

    memmove(str, str + start, strlen(str + start) + 1);

    int len = strlen(str);

    while (len > 0 &&
           (str[len - 1] == ' ' || str[len - 1] == '\t')) {

        str[--len] = '\0';
    }
}

// 목표 시간 설정 (1~600분)
static void set_goal(SetupState* s, int minutes) {

    if (minutes < 1)
        minutes = 1;

    if (minutes > 600)
        minutes = 600;

    snprintf(s->goal, sizeof(s->goal),
             "%d", minutes);

    s->error[0] = '\0';
}

int setup_update(SetupState* s, Session* rec) {

    // 과목 추가창이 열린 상태
    if (s->adding_subject) {

        edit_input(
            s->new_subject,
            sizeof(s->new_subject),
            MAX_SUBJECT_LEN, 0
        );

        // 과목 추가 취소
        if (clicked(cancel_rect) ||
            IsKeyPressed(KEY_ESCAPE)) {

            s->adding_subject = 0;
            s->new_subject[0] = '\0';
            s->error[0] = '\0';

            return 0;
        }

        // 과목 추가 완료
        if (clicked(confirm_rect) ||
            IsKeyPressed(KEY_ENTER)) {

            trim_subject(s->new_subject);

            if (add_subject(s)) {
                s->dropdown_open = 0;
            }
        }

        return 0;
    }

    // 드롭다운 열기 및 닫기
    if (clicked(dropdown_rect)) {

        s->dropdown_open = !s->dropdown_open;
        s->active_field = 0;
        s->error[0] = '\0';

        return 0;
    }

    // 과목 선택 목록이 열린 상태
    if (s->dropdown_open) {

        // 마우스 휠 스크롤
        if (CheckCollisionPointRec(
            GetMousePosition(), subject_list_rect)) {

            float wheel = GetMouseWheelMove();

            if (wheel > 0)
                s->scroll--;

            else if (wheel < 0)
                s->scroll++;

            int max_scroll =
                s->subject_count - VISIBLE_SUBJECTS;

            if (max_scroll < 0)
                max_scroll = 0;

            if (s->scroll < 0)
                s->scroll = 0;

            if (s->scroll > max_scroll)
                s->scroll = max_scroll;
        }

        // 과목 선택
        for (int i = 0; i < VISIBLE_SUBJECTS; i++) {

            int index = s->scroll + i;

            if (index >= s->subject_count)
                break;

            Rectangle r = {
                160,
                172 + i * SUBJECT_ROW_HEIGHT,
                480,
                SUBJECT_ROW_HEIGHT
            };

            if (clicked(r)) {

                s->selected_subject = index;

                strcpy(s->subject, s->subjects[index]);

                s->dropdown_open = 0;
                s->error[0] = '\0';

                return 0;
            }
        }

        // 목록 외부를 클릭하면 닫기
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) ||
            IsKeyPressed(KEY_ESCAPE)) {

            s->dropdown_open = 0;
        }

        return 0;
    }

    // 메인 메뉴 복귀
    if (clicked(back_rect) ||
        IsKeyPressed(KEY_ESCAPE))
        return -1;

    // 새로운 과목 추가
    if (clicked(add_rect)) {

        s->adding_subject = 1;
        s->new_subject[0] = '\0';
        s->error[0] = '\0';
        s->active_field = 0;

        return 0;
    }

    // 목표 시간 직접 입력
    if (clicked(goal_rect)) {
        s->active_field = 2;
        s->error[0] = '\0';
    }
    else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        s->active_field = 0;
    }

    if (s->active_field == 2) {

        edit_input(
            s->goal,
            sizeof(s->goal),
            3, 1
        );
    }

    // 목표 시간 5분 감소
    if (clicked(minus_rect)) {

        set_goal(s, atoi(s->goal) - 5);
        s->active_field = 0;
    }

    // 목표 시간 5분 증가
    if (clicked(plus_rect)) {

        set_goal(s, atoi(s->goal) + 5);
        s->active_field = 0;
    }

    // 빠른 시간 선택 버튼
    for (int i = 0; i < 4; i++) {

        Rectangle r = {
            205 + i * 100,
            365,
            90,
            34
        };

        if (clicked(r)) {

            set_goal(s, preset_minutes[i]);
            s->active_field = 0;
        }
    }

    // 공부 시작
    if (clicked(start_rect) ||
        IsKeyPressed(KEY_ENTER)) {

        int goal = atoi(s->goal);

        // 과목 선택 검사
        if (s->selected_subject < 0 ||
            s->selected_subject >= s->subject_count) {

            strcpy(s->error, "과목을 선택하세요.");

            return 0;
        }

        // 목표 시간 검사
        if (goal < 1 || goal > 600) {

            strcpy(s->error, "목표 시간은 1~600분입니다.");

            return 0;
        }

        // 학습 정보 저장
        memset(rec, 0, sizeof(*rec));

        strcpy(rec->user_name, "User");

        strcpy(
            rec->subject,
            s->subjects[s->selected_subject]
        );

        rec->goal_min = goal;

        // 날짜 저장
        time_t now = time(NULL);
        struct tm* t = localtime(&now);

        if (t != NULL)
            strftime(
                rec->date,
                sizeof(rec->date),
                "%Y-%m-%d", t
            );

        return 1;
    }

    return 0;
}

// 학습 계획 화면 출력
void setup_draw(const SetupState* s) {

    char buf[128];

    // 상단 제목
    center_text("0:Tracker", 30, 38, UI_PRIMARY);

    // 뒤로 가기 화살표
    DrawLineEx((Vector2){36, 57},
               (Vector2){64, 57}, 3, UI_PRIMARY);

    DrawLineEx((Vector2){36, 57},
               (Vector2){48, 45}, 3, UI_PRIMARY);

    DrawLineEx((Vector2){36, 57},
               (Vector2){48, 69}, 3, UI_PRIMARY);

    // 과목 선택 안내
    center_text("SELECT SUBJECT", 88, 17, UI_MUTED);

    // 과목 선택 드롭다운
    DrawRectangleRounded(
        dropdown_rect, 0.9f, 16, UI_SURFACE
    );

    const char* selected = "과목을 선택하세요";

    if (s->selected_subject >= 0 &&
        s->selected_subject < s->subject_count)
        selected = s->subjects[s->selected_subject];

    BeginScissorMode(180, 120, 400, 52);
    center_text(selected, 132, 25, UI_TEXT);
    EndScissorMode();

    // 드롭다운 화살표
    DrawLineEx((Vector2){602, 140},
               (Vector2){610, 148}, 3, UI_PRIMARY);

    DrawLineEx((Vector2){610, 148},
               (Vector2){618, 140}, 3, UI_PRIMARY);

    // 새로운 과목 추가
    center_text("+ 새로운 과목 추가",
                183, 20, UI_PRIMARY);

    // 목표 시간
    center_text("목표 공부 시간",
                230, 22, UI_MUTED);

    // 시간 감소 버튼
    DrawCircle(270, 300, 23, UI_SURFACE);
    DrawLineEx((Vector2){260, 300},
               (Vector2){280, 300}, 3, UI_PRIMARY);

    // 시간 증가 버튼
    DrawCircle(530, 300, 23, UI_SURFACE);
    DrawLineEx((Vector2){520, 300},
               (Vector2){540, 300}, 3, UI_PRIMARY);
    DrawLineEx((Vector2){530, 290},
               (Vector2){530, 310}, 3, UI_PRIMARY);

    // 목표 시간 숫자
    center_text(s->goal, 258, 80, UI_PRIMARY);

    // 직접 입력 중인 경우 밑줄 표시
    if (s->active_field == 2)
        DrawLine(325, 340, 475, 340, UI_PRIMARY);

    center_text("MINUTES", 342, 15, UI_MUTED);

    // 빠른 시간 선택 버튼
    for (int i = 0; i < 4; i++) {

        Rectangle r = {
            205 + i * 100, 365, 90, 34
        };

        int active = atoi(s->goal) == preset_minutes[i];

        DrawRectangleRounded(
            r, 1.0f, 12,
            active ? UI_HOVER : UI_SURFACE
        );

        snprintf(buf, sizeof(buf),
                 "%d분", preset_minutes[i]);

        Vector2 t = MeasureTextEx(
            gui_font, buf, 18, UI_FONT_SPACING
        );

        text(buf,
             r.x + (r.width - t.x) / 2,
             r.y + (r.height - t.y) / 2,
             18, UI_PRIMARY);
    }

    // 공부 시작 버튼
    DrawRectangleRounded(
        start_rect, 0.4f, 12,
        CheckCollisionPointRec(
            GetMousePosition(), start_rect
        ) ? UI_PRIMARY_DARK : UI_PRIMARY
    );

    center_text("공부 시작", 428, 26, UI_WHITE);

    // 오류 메시지
    if (s->error[0] != '\0')
        center_text(s->error, 400, 16, UI_DISTRACT);

    // 드롭다운 목록
    if (s->dropdown_open) {

        DrawRectangleRounded(
            subject_list_rect, 0.08f, 10, UI_SURFACE
        );

        DrawRectangleLinesEx(
            subject_list_rect, 1, UI_BORDER
        );

        if (s->subject_count == 0)
            center_text("등록된 과목이 없습니다.",
                        250, 22, UI_MUTED);

        for (int i = 0; i < VISIBLE_SUBJECTS; i++) {

            int index = s->scroll + i;

            if (index >= s->subject_count)
                break;

            Rectangle r = {
                160,
                172 + i * SUBJECT_ROW_HEIGHT,
                480,
                SUBJECT_ROW_HEIGHT
            };

            if (index == s->selected_subject ||
                CheckCollisionPointRec(GetMousePosition(), r))
                DrawRectangleRec(r, UI_HOVER);

            BeginScissorMode(
                (int)r.x, (int)r.y,
                (int)r.width, (int)r.height
            );

            text(s->subjects[index],
                 r.x + 20, r.y + 9,
                 23, UI_TEXT);

            EndScissorMode();
        }
    }

    // 새로운 과목 추가 팝업
    if (s->adding_subject) {

        // 배경 어둡게 처리
        DrawRectangle(
            0, 0, UI_WIDTH, UI_HEIGHT,
            (Color){0, 0, 0, 100}
        );

        // 팝업 배경
        DrawRectangleRounded(
            (Rectangle){120, 120, 560, 270},
            0.12f, 12, UI_BG
        );

        center_text("새로운 과목 추가",
                    145, 30, UI_PRIMARY);

        input_box(
            new_subject_rect,
            s->new_subject, 1
        );

        button(cancel_rect, "취소");
        button(confirm_rect, "추가");

        if (s->error[0] != '\0')
            center_text(s->error,
                        345, 18, UI_DISTRACT);
    }
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

    float progress = 0.0f;

    if (goal_min > 0)
        progress = (float)study_sec / (goal_min * 60);

    if (progress > 1.0f)
        progress = 1.0f;

    // 과목명
    text(subject, 36, 22, 26, UI_TEXT);

    // 학습 진행 상태
    DrawRectangleRounded(
        (Rectangle){612, 23, 152, 32},
        1.0f, 20,
        (Color){228, 239, 232, 255}
    );

    DrawCircle(629, 39, 4.5f, UI_FOCUS);
    text("학습 진행 중", 653, 29, 17, UI_FOCUS);

    // 원형 진행률 그래프
    Vector2 center = {400, 212};

    float innerRadius = 158.5f;
    float outerRadius = 181.5f;

    float startAngle = 135.0f;
    float endAngle = 405.0f;

    Color trackColor = {234, 220, 226, 255};

    // 전체 진행률 배경
    DrawRing(
        center,
        innerRadius, outerRadius,
        startAngle, endAngle,
        120, trackColor
    );

    // 배경 그래프 끝부분 둥글게 처리
    DrawCircle(280, 332, 11.5f, trackColor);
    DrawCircle(520, 332, 11.5f, trackColor);

    // 실제 공부 진행률
    if (progress > 0.0f) {
        float angle = startAngle + 270.0f * progress;

        DrawRing(
            center,
            innerRadius, outerRadius,
            startAngle, angle,
            120, UI_PRIMARY
        );

        DrawCircle(280, 332, 11.5f, UI_PRIMARY);

        float rad = angle * DEG2RAD;

        DrawCircleV(
            (Vector2){
                center.x + 170.0f * cosf(rad),
                center.y + 170.0f * sinf(rad)
            },
            11.5f,
            UI_PRIMARY
        );
    }

    // 순수 공부 시간
    center_text("FOCUS TIME", 136, 19, UI_MUTED);

    format_time(study_sec, buf, sizeof(buf));
    center_text(buf, 172, 62, UI_PRIMARY);

    // 목표 공부 시간
    snprintf(buf, sizeof(buf), "목표 %d분", goal_min);
    center_text(buf, 260, 20, UI_MUTED);

    // 하단 정보
    const char* labels[] = {
        "총 경과 시간",
        "이탈 시간",
        "이탈 횟수"
    };

    float centers[] = {160, 400, 640};

    for (int i = 0; i < 3; i++) {
        Vector2 t = MeasureTextEx(
            gui_font, labels[i], 17, UI_FONT_SPACING
        );

        text(
            labels[i],
            centers[i] - t.x / 2,
            350, 17, UI_MUTED
        );

        if (i == 0)
            format_time(elapsed, buf, sizeof(buf));
        else if (i == 1)
            format_time(distract_sec, buf, sizeof(buf));
        else
            snprintf(buf, sizeof(buf), "%d회", distract_count);

        t = MeasureTextEx(
            gui_font, buf, 26, UI_FONT_SPACING
        );

        text(
            buf,
            centers[i] - t.x / 2,
            376, 26, UI_TEXT
        );
    }

    // 정보 사이 구분선
    DrawLine(280, 354, 280, 401, UI_BORDER);
    DrawLine(520, 354, 520, 401, UI_BORDER);
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



