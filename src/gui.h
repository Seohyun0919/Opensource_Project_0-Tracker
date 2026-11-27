#ifndef GUI_H
#define GUI_H

#include "raylib.h"
#include "common.h"

// 학습 계획 입력 상태
typedef struct {
    char subject[MAX_SUBJECT_LEN * 3 + 1];
    char goal[4];
    int active_field;
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

#endif