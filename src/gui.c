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
static RenderTexture2D gui_target;
static int gui_expanded = 0;

void gui_init(Font font) {

    gui_font = font;

    gui_target = LoadRenderTexture(
        UI_WIDTH, UI_HEIGHT
    );

    SetTextureFilter(
        gui_target.texture,
        TEXTURE_FILTER_BILINEAR
    );
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
    Vector2 t;

    int elapsed = study_sec + distract_sec;
    int net = earn - penalty + bonus;

    float progress = goal_min > 0
        ? (float)study_sec / (goal_min * 60) : 0.0f;

    float focus = elapsed > 0
        ? (float)study_sec / elapsed : 0.0f;

    int achieved = progress >= 1.0f;
    int percent = (int)(progress * 100);

    if (progress > 1.0f)
        progress = 1.0f;

    // 상단 아이콘
    DrawCircle(400, 47, 27, UI_HOVER);

    if (achieved) {
        DrawLineEx(
            (Vector2){389, 47},
            (Vector2){397, 55},
            4.5f, UI_PRIMARY
        );

        DrawLineEx(
            (Vector2){397, 55},
            (Vector2){412, 39},
            4.5f, UI_PRIMARY
        );
    }
    else {    
        DrawLineEx((Vector2){389, 47},               
        (Vector2){397, 55}, 4.5f, UI_PRIMARY);    
        
        DrawLineEx((Vector2){397, 55},               
        (Vector2){412, 39}, 4.5f, UI_PRIMARY);
    }

    // 상단 제목 및 과목명
    center_text(
        achieved ? "목표를 달성했어요!"
                 : "오늘도 수고했어요!",
        80, 42, UI_PRIMARY
    );

    center_text(subject, 128, 23, UI_MUTED);

    // 왼쪽 카드 배경
    DrawRectangleRounded(
        (Rectangle){36, 166, 354, 218},
        0.22f, 20, UI_SURFACE
    );

    // 오른쪽 카드 배경
    DrawRectangleRounded(
        (Rectangle){406, 166, 358, 218},
        0.22f, 20, UI_SURFACE
    );

    // ===== 왼쪽 카드 =====

    // 순수 공부 시간 제목
    t = MeasureTextEx(
        gui_font, "순수 공부 시간",
        23, UI_FONT_SPACING
    );

    text(
        "순수 공부 시간",
        213 - t.x / 2, 182,
        23, UI_MUTED
    );

    // 순수 공부 시간 숫자
    format_time(study_sec, buf, sizeof(buf));

    t = MeasureTextEx(
        gui_font, buf, 65, UI_FONT_SPACING
    );

    text(
        buf, 213 - t.x / 2, 214,
        65, UI_PRIMARY
    );

    // 목표 달성률 막대 배경
    DrawRectangleRounded(
        (Rectangle){74, 287, 278, 12},
        1.0f, 20, UI_HOVER
    );

    // 실제 목표 달성률
    if (progress > 0.0f) {
        DrawRectangleRounded(
            (Rectangle){74, 287, 278 * progress, 12},
            1.0f, 20, UI_PRIMARY
        );
    }

    // 목표 시간 비교
    snprintf(
        buf, sizeof(buf),
        "%d분 / %d분",
        study_sec / 60, goal_min
    );

    t = MeasureTextEx(
        gui_font, buf, 21, UI_FONT_SPACING
    );

    text(
        buf, 213 - t.x / 2, 307,
        21, UI_MUTED
    );

    // 목표 달성률 숫자
    snprintf(
        buf, sizeof(buf),
        "목표 달성률 %d%%", percent
    );

    t = MeasureTextEx(
        gui_font, buf, 23, UI_FONT_SPACING
    );

    text(
        buf, 213 - t.x / 2, 339,
        23, UI_PRIMARY
    );

    // ===== 오른쪽 카드 =====

    // 원형 집중률 그래프
    Vector2 ringCenter = {503, 269};

    DrawRing(
        ringCenter,
        57.5f, 68.5f,
        -90, 270,
        120, UI_HOVER
    );

    if (focus > 0.0f) {
        DrawRing(
            ringCenter,
            57.5f, 68.5f,
            -90, -90 + 360 * focus,
            120, UI_PRIMARY
        );
    }

    // 집중률 숫자
    snprintf(
        buf, sizeof(buf),
        "%.0f%%", focus * 100
    );

    t = MeasureTextEx(
        gui_font, buf, 46, UI_FONT_SPACING
    );

    text(
        buf, 503 - t.x / 2, 233,
        46, UI_PRIMARY
    );

    // 집중률 제목
    t = MeasureTextEx(
        gui_font, "집중률",
        20, UI_FONT_SPACING
    );

    text(
        "집중률", 503 - t.x / 2, 280,
        20, UI_MUTED
    );

    // 가운데 구분선
    DrawLine(585, 196, 585, 351, UI_BORDER);

    // 획득 포인트 제목
    t = MeasureTextEx(
        gui_font, "획득 포인트",
        21, UI_FONT_SPACING
    );

    text(
        "획득 포인트",
        674 - t.x / 2, 207,
        21, UI_MUTED
    );

    // 획득 포인트 숫자
    snprintf(buf, sizeof(buf), "%+d", net);

    t = MeasureTextEx(
        gui_font, buf, 44, UI_FONT_SPACING
    );

    text(
        buf, 674 - t.x / 2, 237,
        44, UI_PRIMARY
    );

    // POINTS
    t = MeasureTextEx(
        gui_font, "POINTS",
        22, UI_FONT_SPACING
    );

    text(
        "POINTS", 674 - t.x / 2, 282,
        22, UI_PRIMARY
    );

    // 누적 포인트
    if (total_points >= 0)
        snprintf(
            buf, sizeof(buf),
            "누적 %d P", total_points
        );
    else
        snprintf(
            buf, sizeof(buf),
            "포인트 저장 실패"
        );

    t = MeasureTextEx(
        gui_font, buf, 19, UI_FONT_SPACING
    );

    text(
        buf, 674 - t.x / 2, 327,
        19, UI_MUTED
    );

    // ===== 하단 버튼 =====

    // 상세 보기 버튼
    DrawRectangleRounded(
        (Rectangle){36, 420, 354, 52},
        0.25f, 20, UI_SURFACE
    );

    t = MeasureTextEx(
        gui_font, "상세 보기",
        27, UI_FONT_SPACING
    );

    text(
        "상세 보기",
        213 - t.x / 2,
        446 - t.y / 2,
        27, UI_TEXT
    );

    // 메인으로 버튼
    DrawRectangleRounded(
        (Rectangle){410, 420, 354, 52},
        0.25f, 20, UI_PRIMARY
    );

    t = MeasureTextEx(
        gui_font, "메인으로",
        27, UI_FONT_SPACING
    );

    text(
        "메인으로",
        587 - t.x / 2,
        446 - t.y / 2,
        27, UI_WHITE
    );

    (void)distract_count;
}


void draw_detail_screen(
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
    Vector2 t;

    int elapsed = study_sec + distract_sec;
    int net = earn - penalty + bonus;

    float focus = elapsed > 0
        ? (float)study_sec / elapsed * 100 : 0.0f;

    // 상단 제목
    center_text("상세 학습 기록", 25, 40, UI_PRIMARY);
    center_text(subject, 75, 23, UI_MUTED);

    // 카드 배경
    DrawRectangleRounded(
        (Rectangle){36, 110, 354, 284},
        0.18f, 20, UI_SURFACE
    );

    DrawRectangleRounded(
        (Rectangle){406, 110, 358, 284},
        0.18f, 20, UI_SURFACE
    );

    // 카드 제목
    text("시간 분석", 60, 127, 27, UI_PRIMARY);
    text("포인트 정산", 430, 127, 27, UI_PRIMARY);

    DrawLine(60, 165, 366, 165, UI_BORDER);
    DrawLine(430, 165, 740, 165, UI_BORDER);

    // 시간 분석
    const char* timeLabels[] = {
        "목표 시간",
        "총 경과 시간",
        "순수 공부 시간",
        "학습 이탈 시간",
        "학습 이탈 횟수",
        "집중률"
    };

    for (int i = 0; i < 6; i++) {
        int y = 178 + i * 34;

        text(timeLabels[i], 60, y, 22, UI_MUTED);

        switch (i) {
            case 0:
                format_time(goal_min * 60, buf, sizeof(buf));
                break;
            case 1:
                format_time(elapsed, buf, sizeof(buf));
                break;
            case 2:
                format_time(study_sec, buf, sizeof(buf));
                break;
            case 3:
                format_time(distract_sec, buf, sizeof(buf));
                break;
            case 4:
                snprintf(buf, sizeof(buf), "%d회", distract_count);
                break;
            case 5:
                snprintf(buf, sizeof(buf), "%.1f%%", focus);
                break;
        }

        t = MeasureTextEx(gui_font, buf, 23, UI_FONT_SPACING);
        text(buf, 366 - t.x, y, 23, UI_TEXT);
    }

    // 포인트 정산
    const char* pointLabels[] = {
        "학습 포인트",
        "이탈 차감",
        "보너스"
    };

    int points[] = {earn, -penalty, bonus};

    for (int i = 0; i < 3; i++) {
        int y = 178 + i * 38;

        text(pointLabels[i], 430, y, 22, UI_MUTED);

        snprintf(buf, sizeof(buf), "%+d P", points[i]);

        t = MeasureTextEx(gui_font, buf, 24, UI_FONT_SPACING);

        text(buf, 740 - t.x, y, 24,
             i == 1 ? UI_DISTRACT : UI_TEXT);
    }

    DrawLine(430, 290, 740, 290, UI_BORDER);

    // 최종 획득 포인트
    text("최종 획득 포인트", 430, 305, 22, UI_PRIMARY);

    snprintf(buf, sizeof(buf), "%+d P", net);

    t = MeasureTextEx(gui_font, buf, 28, UI_FONT_SPACING);
    text(buf, 740 - t.x, 302, 28, UI_PRIMARY);

    // 누적 포인트
    text("누적 포인트", 430, 350, 22, UI_MUTED);

    if (total_points >= 0)
        snprintf(buf, sizeof(buf), "%d P", total_points);
    else
        snprintf(buf, sizeof(buf), "저장 실패");

    t = MeasureTextEx(gui_font, buf, 24, UI_FONT_SPACING);
    text(buf, 740 - t.x, 350, 24, UI_TEXT);

    // 결과 요약 버튼
    DrawRectangleRounded(
        (Rectangle){36, 420, 354, 52},
        0.25f, 20, UI_SURFACE
    );

    t = MeasureTextEx(
        gui_font, "결과 요약", 27, UI_FONT_SPACING
    );

    text("결과 요약",
         213 - t.x / 2, 446 - t.y / 2,
         27, UI_TEXT);

    // 메인으로 버튼
    DrawRectangleRounded(
        (Rectangle){410, 420, 354, 52},
        0.25f, 20, UI_PRIMARY
    );

    t = MeasureTextEx(
        gui_font, "메인으로", 27, UI_FONT_SPACING
    );

    text("메인으로",
         587 - t.x / 2, 446 - t.y / 2,
         27, UI_WHITE);
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

// ===== 학습 기록 화면 =====

static SetupState history_subjects;

static Session* history_list = NULL;
static int history_count = 0;

static int history_page = 0;
static int history_selected = -1;

static int history_subject_scroll = 0;
static int history_record_scroll = 0;

// 학습 기록 메모리 해제
void history_close(void) {
    free(history_list);
    history_list = NULL;
    history_count = 0;
}

// 학습 기록 초기화
void history_init(void) {

    history_close();

    // 등록된 과목 불러오기
    setup_init(&history_subjects);

    // 화면 상태 초기화
    history_page = 0;
    history_selected = -1;

    history_subject_scroll = 0;
    history_record_scroll = 0;

    // 전체 학습 기록 개수 확인
    int count = loadSessions(NULL, 0);

    if (count <= 0)
        return;

    // 학습 기록 저장 공간 할당
    history_list = malloc(sizeof(Session) * count);

    if (history_list == NULL)
        return;

    // 학습 기록 불러오기
    history_count = loadSessions(history_list, count);

    if (history_count > count)
        history_count = count;
}

// 과목별 학습 기록 개수 계산
static int history_subject_count(const char* subject) {

    int count = 0;

    for (int i = 0; i < history_count; i++) {

        if (strcmp(history_list[i].subject, subject) == 0)
            count++;
    }

    return count;
}


// 과목 선택 화면 출력
static void history_draw_subjects(void) {

    char buf[128];

    // 상단 뒤로 가기 화살표
    DrawLineEx((Vector2){39, 48},
               (Vector2){65, 48}, 3, UI_PRIMARY);

    DrawLineEx((Vector2){39, 48},
               (Vector2){51, 36}, 3, UI_PRIMARY);

    DrawLineEx((Vector2){39, 48},
               (Vector2){51, 60}, 3, UI_PRIMARY);

    // 상단 제목
    center_text("학습 기록", 24, 44, UI_PRIMARY);
    center_text("과목을 선택하세요", 85, 24, UI_MUTED);

    // 등록된 과목이 없는 경우
    if (history_subjects.subject_count == 0) {
        center_text("등록된 과목이 없습니다.",
                    220, 28, UI_MUTED);
        return;
    }

    // 한 화면에 최대 4개의 과목 표시
    for (int i = 0; i < 4; i++) {

        int index = history_subject_scroll + i;

        if (index >= history_subjects.subject_count)
            break;

        int y = 132 + i * 80;

        Rectangle r = {36, y, 728, 64};

        int hover = CheckCollisionPointRec(
            GetMousePosition(), r
        );

        // 과목 카드 배경
        DrawRectangleRounded(
            r, 0.25f, 20,
            hover ? UI_HOVER : UI_SURFACE
        );

        // 책 아이콘
        DrawRectangleLinesEx(
            (Rectangle){58, y + 18, 21, 28},
            2.5f, UI_PRIMARY
        );

        DrawRectangleLinesEx(
            (Rectangle){79, y + 18, 21, 28},
            2.5f, UI_PRIMARY
        );

        DrawLine(64, y + 26, 74, y + 26, UI_PRIMARY);
        DrawLine(64, y + 33, 74, y + 33, UI_PRIMARY);
        DrawLine(84, y + 26, 94, y + 26, UI_PRIMARY);
        DrawLine(84, y + 33, 94, y + 33, UI_PRIMARY);

        // 과목명
        text(
            history_subjects.subjects[index],
            128, y + 17, 28, UI_TEXT
        );

        // 과목별 기록 개수
        int count = history_subject_count(
            history_subjects.subjects[index]
        );

        snprintf(buf, sizeof(buf), "%d개의 기록", count);

        Vector2 t = MeasureTextEx(
            gui_font, buf, 22, UI_FONT_SPACING
        );

        text(buf, 702 - t.x, y + 20, 22, UI_MUTED);

        // 오른쪽 화살표
        DrawLineEx(
            (Vector2){728, y + 23},
            (Vector2){737, y + 32},
            2.5f, UI_PRIMARY
        );

        DrawLineEx(
            (Vector2){737, y + 32},
            (Vector2){728, y + 41},
            2.5f, UI_PRIMARY
        );
    }

    // 스크롤바
    if (history_subjects.subject_count > 4) {

        float height =
            304.0f * 4 / history_subjects.subject_count;

        float position =
            304.0f * history_subject_scroll
            / history_subjects.subject_count;

        DrawRectangleRounded(
            (Rectangle){779, 132, 6, 304},
            1.0f, 10, UI_BORDER
        );

        DrawRectangleRounded(
            (Rectangle){779, 132 + position, 6, height},
            1.0f, 10, UI_PRIMARY
        );
    }
}

static void history_calculate(void);
static void history_draw_analysis(void);
static void history_draw_records(void);
static int history_compare(const void* a, const void* b);
static void history_draw_performance(void);

// 학습 기록 화면 입력 처리

int history_update(void) {

    Rectangle back = {24, 24, 55, 50};

    // 뒤로 가기
    if (clicked(back) || IsKeyPressed(KEY_ESCAPE)) {

        if (history_page == 0)
            return -1;

        history_page--;
        return 0;
    }

    // 과목 선택 화면
    if (history_page == 0) {

        float wheel = GetMouseWheelMove();

        if (wheel > 0)
            history_subject_scroll--;
        else if (wheel < 0)
            history_subject_scroll++;

        int max_scroll =
            history_subjects.subject_count - 4;

        if (max_scroll < 0)
            max_scroll = 0;

        if (history_subject_scroll < 0)
            history_subject_scroll = 0;

        if (history_subject_scroll > max_scroll)
            history_subject_scroll = max_scroll;

        // 과목 선택
        for (int i = 0; i < 4; i++) {

            int index = history_subject_scroll + i;

            if (index >= history_subjects.subject_count)
                break;

            Rectangle r = {
                36, 132 + i * 80, 728, 64
            };

            if (clicked(r)) {

                history_selected = index;
                history_record_scroll = 0;

                history_calculate();

                history_page = 1;
                break;
            }
        }
    }

    // 과목별 학습 분석 화면
    else if (history_page == 1) {

        Rectangle detail = {36, 415, 728, 49};

        if (clicked(detail)) {

            // 날짜 오름차순 정렬
            if (history_count > 1) {

                qsort(
                    history_list,
                    history_count,
                    sizeof(Session),
                    history_compare
                );
            }

            history_record_scroll = 0;
            history_page = 2;
        }
    }

    // 상세 학습 기록 화면
    else if (history_page == 2) {

        float wheel = GetMouseWheelMove();

        if (wheel > 0)
            history_record_scroll--;
        else if (wheel < 0)
            history_record_scroll++;

        int max_scroll =
            history_subject_count(
                history_subjects.subjects[history_selected]
            ) - 5;

        if (max_scroll < 0)
            max_scroll = 0;

        if (history_record_scroll < 0)
            history_record_scroll = 0;

        if (history_record_scroll > max_scroll)
            history_record_scroll = max_scroll;
    }

    return 0;
}


// 학습 기록 화면 출력
void history_draw(void) {

    if (history_page == 0) {
        history_draw_subjects();
    }

    else if (history_page == 1) {
        history_draw_analysis();
    }

    else if (history_page == 2) {
        history_draw_records();
    }
}

// 과목별 학습 통계
typedef struct {
    long long total_study;
    long long total_distract;

    int session_count;
    int study_days;
    int achieved_count;

    double average_sec;
    long long best_sec;

    float focus_rate;
    float achievement_rate;

    long long recent_sec[7];
    char recent_dates[7][11];

} HistoryStats;

static HistoryStats history_stats;

// 특정 과목의 특정 날짜 공부 시간 합산
static long long history_daily_total(
    const char* subject,
    const char* date
) {
    long long total = 0;

    for (int i = 0; i < history_count; i++) {

        if (strcmp(history_list[i].subject, subject) == 0 &&
            strcmp(history_list[i].date, date) == 0) {

            total += history_list[i].study_sec;
        }
    }

    return total;
}


// 선택한 과목의 학습 통계 계산
static void history_calculate(void) {

    memset(&history_stats, 0, sizeof(history_stats));

    if (history_selected < 0 ||
        history_selected >= history_subjects.subject_count)
        return;

    const char* subject =
        history_subjects.subjects[history_selected];

    for (int i = 0; i < history_count; i++) {

        Session* s = &history_list[i];

        // 선택한 과목만 계산
        if (strcmp(s->subject, subject) != 0)
            continue;

        // 전체 학습 시간 및 횟수
        history_stats.total_study += s->study_sec;
        history_stats.total_distract += s->distract_sec;
        history_stats.session_count++;

        // 목표 달성 횟수
        if (s->goal_achieved)
            history_stats.achieved_count++;

        // 이미 계산한 날짜인지 확인
        int duplicate = 0;

        for (int j = 0; j < i; j++) {

            if (strcmp(history_list[j].subject, subject) == 0 &&
                strcmp(history_list[j].date, s->date) == 0) {

                duplicate = 1;
                break;
            }
        }

        // 같은 날짜는 한 번만 계산
        if (!duplicate) {

            long long daily =
                history_daily_total(subject, s->date);

            // 실제 공부한 날짜만 집계
            if (daily > 0)
                history_stats.study_days++;

            // 역대 최고 일일 공부 시간
            if (daily > history_stats.best_sec)
                history_stats.best_sec = daily;
        }
    }

    // 일평균 공부 시간
    if (history_stats.study_days > 0) {

        history_stats.average_sec =
            (double)history_stats.total_study
            / history_stats.study_days;
    }

    // 전체 집중률
    long long elapsed =
        history_stats.total_study +
        history_stats.total_distract;

    if (elapsed > 0) {

        history_stats.focus_rate =
            100.0 * history_stats.total_study
            / elapsed;
    }

    // 목표 달성률
    if (history_stats.session_count > 0) {

        history_stats.achievement_rate =
            100.0 * history_stats.achieved_count
            / history_stats.session_count;
    }

    // 최근 7일간 공부 시간
    time_t now = time(NULL);
    struct tm* local = localtime(&now);

    if (local == NULL)
        return;

    struct tm today = *local;

    for (int i = 0; i < 7; i++) {

        struct tm day = today;

        day.tm_mday -= (6 - i);
        day.tm_isdst = -1;

        if (mktime(&day) == (time_t)-1)
            continue;

        strftime(
            history_stats.recent_dates[i],
            sizeof(history_stats.recent_dates[i]),
            "%Y-%m-%d",
            &day
        );

        history_stats.recent_sec[i] =
            history_daily_total(
                subject,
                history_stats.recent_dates[i]
            );
    }
}

// 수평 점선 출력
static void history_dashed_line(
    float x1, float x2, float y, Color color
) {
    for (float x = x1; x < x2; x += 10) {

        float end = x + 6;

        if (end > x2)
            end = x2;

        DrawLineEx(
            (Vector2){x, y},
            (Vector2){end, y},
            1.8f, color
        );
    }
}
// Y축 눈금 간격 계산
static double history_axis_step(double best_min) {

    if (best_min <= 60)
        return 15;

    if (best_min <= 120)
        return 30;

    if (best_min <= 240)
        return 60;

    if (best_min <= 480)
        return 120;

    return ceil(best_min / 240.0) * 60;
}

// 최근 7일 공부 시간 그래프
static void history_draw_chart(void) {

    char buf[64];
    Vector2 t;

    Color bar_color = {201, 151, 170, 255};

    // 그래프 카드
    DrawRectangleRounded(
        (Rectangle){36, 201, 452, 195},
        0.15f, 20, UI_SURFACE
    );

    text("최근 7일 공부 시간",
         55, 211, 23, UI_TEXT);

    // 그래프 영역
    float left = 93;
    float right = 462;
    float top = 258;
    float bottom = 357;

    float height = bottom - top;
    float width = right - left;

    // 전체 평균과 역대 최고 기록
    double best = history_stats.best_sec / 60.0;
    double average = history_stats.average_sec / 60.0;

    // Y축 자동 조절
    double step = history_axis_step(best);
    double max_value = ceil(best / step) * step;

    if (max_value < step * 2)
        max_value = step * 2;

    // Y축 눈금 및 가로선
    for (double value = 0;
         value <= max_value;
         value += step) {

        float y = bottom -
            (float)(value / max_value) * height;

        snprintf(buf, sizeof(buf), "%.0f", value);

        t = MeasureTextEx(
            gui_font, buf, 15, UI_FONT_SPACING
        );

        text(buf, left - 10 - t.x,
             y - 8, 15, UI_MUTED);

        DrawLineEx(
            (Vector2){left, y},
            (Vector2){right, y},
            1, UI_BORDER
        );
    }

    text("(단위:분)", 200, 220, 14, UI_MUTED);

    // 최근 7일 막대그래프
    float gap = width / 7;

    for (int i = 0; i < 7; i++) {

        double minutes =
            history_stats.recent_sec[i] / 60.0;

        float bar_height =
            (float)(minutes / max_value) * height;

        float cx = left + gap * (i + 0.5f);

        // 역대 최고 학습일은 진한 버건디
        Color color = bar_color;

        if (history_stats.best_sec > 0 &&
            history_stats.recent_sec[i] ==
            history_stats.best_sec) {

            color = UI_PRIMARY;
        }

        if (bar_height > 0) {

            DrawRectangleRounded(
                (Rectangle){
                    cx - 13,
                    bottom - bar_height,
                    26,
                    bar_height
                },
                0.2f, 10, color
            );
        }

        // 날짜 표시
        int month = 0;
        int day = 0;

        sscanf(
            history_stats.recent_dates[i],
            "%*d-%d-%d",
            &month, &day
        );

        snprintf(
            buf, sizeof(buf), "%d/%d",
            month, day
        );

        t = MeasureTextEx(
            gui_font, buf, 15, UI_FONT_SPACING
        );

        text(buf, cx - t.x / 2,
             365, 15, UI_MUTED);
    }

    // 공부 기록이 없으면 점선 생략
    if (history_stats.best_sec <= 0)
        return;

    // 전체 평균 점선
    float average_y = bottom -
        (float)(average / max_value) * height;

    history_dashed_line(
        left, right, average_y, UI_FOCUS
    );

    // 역대 최고 점선
    float best_y = bottom -
        (float)(best / max_value) * height;

    history_dashed_line(
        left, right, best_y, UI_WARNING
    );

    // 상단 Best 범례
    history_dashed_line(270, 288, 221, UI_WARNING);

    snprintf(buf, sizeof(buf),"Best %.1f", best);

    text(buf, 293, 213, 15, UI_WARNING);

    // 상단 평균 범례
    history_dashed_line(374, 392, 221, UI_FOCUS);


    snprintf(buf, sizeof(buf),"평균 %.1f", average);

    text(buf, 397, 213, 15, UI_FOCUS);
    
}


// 과목별 학습 분석 화면
static void history_draw_analysis(void) {

    char buf[128];
    Vector2 t;

    // 뒤로 가기 화살표
    DrawLineEx((Vector2){39, 48},
               (Vector2){65, 48}, 3, UI_PRIMARY);

    DrawLineEx((Vector2){39, 48},
               (Vector2){51, 36}, 3, UI_PRIMARY);

    DrawLineEx((Vector2){39, 48},
               (Vector2){51, 60}, 3, UI_PRIMARY);

    // 상단 과목명 및 제목
    center_text(
        history_subjects.subjects[history_selected],
        24, 42, UI_PRIMARY
    );

    center_text("학습 분석", 66, 22, UI_MUTED);

    // 통계 카드 제목
    const char* labels[] = {
        "누적 공부 시간",
        "일평균 공부 시간",
        "총 학습 일수"
    };

    // 통계 카드 3개
    for (int i = 0; i < 3; i++) {

        float x = 36 + i * 248;
        float cx = x + 116;

        // 카드 배경
        DrawRectangleRounded(
            (Rectangle){x, 101, 232, 86},
            0.25f, 20, UI_SURFACE
        );

        // 카드 제목
        t = MeasureTextEx(
            gui_font, labels[i], 22, UI_FONT_SPACING
        );

        text(labels[i], cx - t.x / 2,
             111, 22, UI_MUTED);

        // 카드별 통계 수치
        if (i == 0) {

            long long minutes =
                history_stats.total_study / 60;

            snprintf(
                buf, sizeof(buf),
                "%lld시간 %lld분",
                minutes / 60, minutes % 60
            );
        }

        else if (i == 1) {

            snprintf(
                buf, sizeof(buf),
                "%.1f분",
                history_stats.average_sec / 60.0
            );
        }

        else {

            snprintf(
                buf, sizeof(buf),
                "%d일",
                history_stats.study_days
            );
        }

        // 통계 수치 가운데 정렬
        float size = 36;

        t = MeasureTextEx(
            gui_font, buf, size, UI_FONT_SPACING
        );

        // 글자가 카드 너비를 초과하면 크기 조절
        while (t.x > 208 && size > 12) {

            size--;

            t = MeasureTextEx(
                gui_font, buf, size, UI_FONT_SPACING
            );
        }

        text(buf, cx - t.x / 2,
             141, size, UI_PRIMARY);
    }
    history_draw_chart();
    history_draw_performance();
    // 상세 학습 기록 버튼
    Rectangle detail = {36, 415, 728, 49};

    int hover = CheckCollisionPointRec(
        GetMousePosition(), detail
    );

    DrawRectangleRounded(
        detail, 0.25f, 20,
        hover ? UI_PRIMARY_DARK : UI_PRIMARY
    );

    center_text("상세 학습 기록",
                425, 27, UI_WHITE);

    DrawLineEx((Vector2){728, 432},
               (Vector2){737, 440}, 2.5f, UI_WHITE);

    DrawLineEx((Vector2){737, 440},
               (Vector2){728, 448}, 2.5f, UI_WHITE);    
}

// 원형 그래프 출력
static void history_draw_ring(
    float x, float y,
    float rate, const char* label
) {
    char buf[32];
    Vector2 center = {x, y};

    if (rate < 0)
        rate = 0;

    if (rate > 100)
        rate = 100;

    // 원형 그래프 배경
    DrawRing(
        center,
        40, 50,
        0, 360, 100,
        UI_HOVER
    );

    // 비율에 따른 진행률 표시
    if (rate > 0) {

        DrawRing(
            center,
            40, 50,
            -90,
            -90 + 360 * rate / 100,
            100, UI_PRIMARY
        );

        // 진행률 양쪽 끝을 둥글게 처리
        float angle =
            (-90 + 360 * rate / 100) * DEG2RAD;

        DrawCircle(
            x, y - 45,
            5, UI_PRIMARY
        );

        DrawCircle(
            x + 45 * cosf(angle),
            y + 45 * sinf(angle),
            5, UI_PRIMARY
        );
    }

    // 중앙 퍼센트 표시
    snprintf(buf, sizeof(buf), "%.0f%%", rate);

    Vector2 t = MeasureTextEx(
        gui_font, buf, 32, UI_FONT_SPACING
    );

    text(buf, x - t.x / 2,
         y - 16, 32, UI_PRIMARY);

    // 원형 그래프 하단 제목
    t = MeasureTextEx(
        gui_font, label, 20, UI_FONT_SPACING
    );

    text(label, x - t.x / 2,
         y + 57, 20, UI_MUTED);
}

// 학습 성과 카드
static void history_draw_performance(void) {

    // 카드 배경
    DrawRectangleRounded(
        (Rectangle){502, 201, 262, 195},
        0.15f, 20, UI_SURFACE
    );

    // 카드 제목
    Vector2 t = MeasureTextEx(
        gui_font, "학습 성과",
        23, UI_FONT_SPACING
    );

    text("학습 성과",
         633 - t.x / 2,
         211, 23, UI_TEXT);

    // 집중률
    history_draw_ring(
        568, 301,
        history_stats.focus_rate,
        "집중률"
    );

    // 목표 달성률
    history_draw_ring(
        698, 301,
        history_stats.achievement_rate,
        "목표 달성률"
    );
}

// 학습 기록 날짜 오름차순 정렬
static int history_compare(
    const void* a, const void* b
) {
    const Session* s1 = a;
    const Session* s2 = b;

    int result = strcmp(s1->date, s2->date);

    if (result != 0)
        return result;

    if (s1->session_num < s2->session_num)
        return -1;

    if (s1->session_num > s2->session_num)
        return 1;

    return 0;
}

// 선택한 과목의 n번째 학습 기록
static Session* history_get_record(int index) {

    const char* subject =
        history_subjects.subjects[history_selected];

    int count = 0;

    for (int i = 0; i < history_count; i++) {

        if (strcmp(history_list[i].subject, subject) != 0)
            continue;

        if (count == index)
            return &history_list[i];

        count++;
    }

    return NULL;
}

// 역대 최고 학습일인지 확인
static int history_is_best(const Session* s) {

    if (history_stats.best_sec <= 0)
        return 0;

    return history_daily_total(
        s->subject, s->date
    ) == history_stats.best_sec;
}


// 상세 학습 기록 화면
static void history_draw_records(void) {

    char buf[128];
    Vector2 t;

    Color best_bg = {231, 202, 213, 255};

    // 뒤로 가기 화살표
    DrawLineEx((Vector2){39, 48},
               (Vector2){65, 48}, 3, UI_PRIMARY);

    DrawLineEx((Vector2){39, 48},
               (Vector2){51, 36}, 3, UI_PRIMARY);

    DrawLineEx((Vector2){39, 48},
               (Vector2){51, 60}, 3, UI_PRIMARY);

    // 상단 제목
    center_text("상세 학습 기록",
                24, 42, UI_PRIMARY);

    center_text(
        history_subjects.subjects[history_selected],
        72, 23, UI_MUTED
    );

    // 테이블 배경
    DrawRectangleRounded(
        (Rectangle){30, 112, 740, 306},
        0.12f, 20, UI_SURFACE
    );

    // 열 제목
    const char* labels[] = {
        "날짜",
        "목표 시간",
        "공부 시간",
        "이탈 시간",
        "이탈 횟수",
        "목표 달성"
    };

    float cx[] = {
        103, 239, 350, 461, 572, 700
    };

    for (int i = 0; i < 6; i++) {

        t = MeasureTextEx(
            gui_font, labels[i],
            21, UI_FONT_SPACING
        );

        text(labels[i], cx[i] - t.x / 2,
             130, 21, UI_MUTED);
    }

    DrawLine(44, 160, 756, 160, UI_PRIMARY);

    // 학습 기록이 없는 경우
    if (history_stats.session_count == 0) {

        center_text("저장된 학습 기록이 없습니다.",
                    260, 25, UI_MUTED);
    }

    // 한 화면에 최대 5개 기록 표시
    for (int i = 0; i < 5; i++) {

        Session* s = history_get_record(
            history_record_scroll + i
        );

        if (s == NULL)
            break;

        float y = 162 + i * 50;

        // BEST 학습일 배경 강조
        if (history_is_best(s)) {

            DrawRectangle(
                36, y, 728, 49, best_bg
            );

            DrawRectangle(
                36, y, 5, 49, UI_PRIMARY
            );
        }

        // 날짜 및 학습 수치 출력
        for (int j = 0; j < 5; j++) {

            switch (j) {

                case 0:
                    snprintf(buf, sizeof(buf),
                             "%s", s->date);
                    break;

                case 1:
                    snprintf(buf, sizeof(buf),
                             "%d", s->goal_min);
                    break;

                case 2:
                    snprintf(buf, sizeof(buf),
                             "%d", s->study_sec / 60);
                    break;

                case 3:
                    snprintf(buf, sizeof(buf),
                             "%d", s->distract_sec / 60);
                    break;

                case 4:
                    snprintf(buf, sizeof(buf),
                             "%d", s->distract_count);
                    break;
            }

            t = MeasureTextEx(
                gui_font, buf, 22, UI_FONT_SPACING
            );

            text(buf, cx[j] - t.x / 2,
                 y + 13, 22,
                 j == 2 ? UI_PRIMARY : UI_TEXT);
        }

        // 목표 달성 여부
        const char* status =
            s->goal_achieved ? "달성" : "미달성";

        Color status_bg = s->goal_achieved
            ? (Color){228, 239, 232, 255}
            : UI_HOVER;

        Color status_color = s->goal_achieved
            ? UI_FOCUS : UI_PRIMARY;

        DrawRectangleRounded(
            (Rectangle){663, y + 9, 74, 30},
            1.0f, 20, status_bg
        );

        t = MeasureTextEx(
            gui_font, status, 19, UI_FONT_SPACING
        );

        text(status, 700 - t.x / 2,
             y + 14, 19, status_color);

        // 행 구분선
        if (i < 4)
            DrawLine(44, y + 49,
                     756, y + 49, UI_BORDER);
    }

    // 스크롤바
    if (history_stats.session_count > 5) {

        float height =
            250.0f * 5 / history_stats.session_count;

        float position =
            250.0f * history_record_scroll
            / history_stats.session_count;

        DrawRectangleRounded(
            (Rectangle){779, 162, 6, 250},
            1.0f, 10, UI_BORDER
        );

        DrawRectangleRounded(
            (Rectangle){779, 162 + position, 6, height},
            1.0f, 10, UI_PRIMARY
        );
    }

    // 하단 BEST 기록 안내
    if (history_stats.best_sec > 0) {

        DrawRectangleRounded(
            (Rectangle){36, 438, 16, 16},
            0.3f, 10, best_bg
        );

        snprintf(
            buf, sizeof(buf),
            "BEST · %.1f분",
            history_stats.best_sec / 60.0
        );

        text(buf, 61, 434, 21, UI_PRIMARY);
    }

    // 전체 기록 개수 및 단위
    snprintf(
        buf, sizeof(buf),
        "총 %d개의 기록 · 단위: 분",
        history_stats.session_count
    );

    t = MeasureTextEx(
        gui_font, buf, 20, UI_FONT_SPACING
    );

    text(buf, 764 - t.x,
         434, 20, UI_MUTED);
}

// 학습 기록 화면 확대 여부
int history_is_expanded(void) {
    return history_page == 1 || history_page == 2;
}

// 화면 출력 시작 및 창 크기 조절
void gui_begin(int screen) {

    int expanded =
        screen == 2 && history_is_expanded();

    if (expanded != gui_expanded) {

        gui_expanded = expanded;

        if (gui_expanded) {
            SetWindowSize(1200, 720);
            SetMouseScale(2.0f / 3.0f, 2.0f / 3.0f);
        }
        else {
            SetWindowSize(800, 480);
            SetMouseScale(1.0f, 1.0f);
        }
    }

    BeginTextureMode(gui_target);
}

// 화면 출력 종료
void gui_end(void) {

    EndTextureMode();

    BeginDrawing();

    ClearBackground(UI_BG);

    DrawTexturePro(
        gui_target.texture,
        (Rectangle){0, 0, UI_WIDTH, -UI_HEIGHT},
        (Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()},
        (Vector2){0, 0},
        0, WHITE
    );

    EndDrawing();
}

// GUI 메모리 해제
void gui_close(void) {
    UnloadRenderTexture(gui_target);
}