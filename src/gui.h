#ifndef GUI_H
#define GUI_H

#include "raylib.h"
#include "common.h"

#ifndef MAX_SUBJECTS
#define MAX_SUBJECTS 100
#endif
// 학습 계획 입력 상태
typedef struct {
    char subject[MAX_SUBJECT_LEN * 3 + 1];
    char goal[4];
    int active_field;

    // 과목 목록
    char subjects[MAX_SUBJECTS][MAX_SUBJECT_LEN * 3 + 1];
    int subject_count;
    int selected_subject;

    // 과목 추가
    char new_subject[MAX_SUBJECT_LEN * 3 + 1];
    int adding_subject;

    // 과목 목록 스크롤
    int scroll;
    int dropdown_open;  // 과목 선택 목록 열림 여부

    // 오류 안내
    char error[128];    

} SetupState;

// GUI 초기화 및 종료
void gui_init(Font font);

// 학습 계획 입력 화면
void setup_init(SetupState* s);
int setup_update(SetupState* s, Session* rec);
void setup_draw(const SetupState* s);

// 학습 진행 화면
void draw_study_screen(
    const char* subject,
    int goal_min,
    int study_sec,
    int distract_sec,
    int distract_count
);

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
);

// 이탈 경고 화면
void draw_warning_screen(int keyword_index);

// 기록 저장 실패 안내
void draw_save_error(void);

// 학습 기록 화면
void history_init(void);
int history_update(void);
void history_draw(void);
void history_close(void);
int history_is_expanded(void);

void gui_begin(int screen);
void gui_end(void);
void gui_close(void);



#endif