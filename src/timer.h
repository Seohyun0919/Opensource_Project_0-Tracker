// M1 계획·타이머. M8(main)이 input_plan, run_timer 순서로 부름.
#ifndef TIMER_H
#define TIMER_H
 
#include "common.h"
 
void input_user_name(char *user_name);
void input_subject(char *subject);
 
// 1~600 범위 밖이면 다시 물음. 입력이 끊기면(EOF) 1을 돌려줌
int input_goal_min(void);
 
// 이름·과목·목표 시간·날짜를 rec에 채움. 올바른 값이 들어올 때까지 다시 물음
void input_plan(Session *rec);
 
// 반드시 input_plan 다음에 부를 것 (goal_min이 있어야 달성 판정함)
// Q로 끝나면 study_sec, goal_achieved를 채워 돌아옴
void run_timer(Session *rec);
 
#endif