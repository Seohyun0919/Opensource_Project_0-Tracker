// M1 계획 입력과 경과 시간 계산. 학습 실행은 study_ui가 담당한다.
#ifndef TIMER_H
#define TIMER_H
 
#include "common.h"
 
void input_subject(char *subject);
 
// 1~600 범위 밖이면 다시 물음. 입력이 끊기면(EOF) 1을 돌려줌
int input_goal_min(void);
 
// 이름·과목·목표 시간·날짜를 rec에 채움. 올바른 값이 들어올 때까지 다시 물음
void input_plan(Session *rec);
 
typedef struct {
    double start_time;   // 시작 시각(초). raylib이면 GetTime() 값
    double last_temp_save; // 마지막 임시 저장 시각(초)
    int base_study_sec;      // 이어하기 전에 이미 쌓인 학습 시간
    int base_distract_sec;   // 이어하기 전에 이미 쌓인 이탈 시간
} TimerState;

void start_timer(TimerState* timer, double now);
void update_timer(const TimerState* timer, Session* rec, double now);

// 마지막 저장 후 60초가 지났을 때만 임시 파일에 저장함
void save_temp_if_due(TimerState* timer, const Session* rec, double now);

// 임시 파일에서 불러온 기록 위에 이어서 센다. saved의 학습·이탈 시간이 시작값이 됨
void resume_timer(TimerState* timer, const Session* saved, double now);

#endif
