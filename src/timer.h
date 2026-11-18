// M1 계획 입력과 경과 시간 계산. 학습 실행은 Study_ui가 담당한다.
#ifndef TIMER_H
#define TIMER_H
 
#include "common.h"
#include <windows.h>
 
void input_user_name(char *user_name);
void input_subject(char *subject);
 
// 1~600 범위 밖이면 다시 물음. 입력이 끊기면(EOF) 1을 돌려줌
int input_goal_min(void);
 
// 이름·과목·목표 시간·날짜를 rec에 채움. 올바른 값이 들어올 때까지 다시 물음
void input_plan(Session *rec);
 
typedef struct {
    ULONGLONG start_ms;
} TimerState;

void start_timer(TimerState* timer);
// 이탈 감지 연결 전까지 전체 경과 시간을 공부 시간으로 계산한다.
void update_timer(const TimerState* timer, Session* rec);
 
#endif
