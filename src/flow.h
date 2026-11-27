#ifndef FLOW_H
#define FLOW_H

#include "common.h"
#include "timer.h"



typedef enum { FLOW_STUDY, FLOW_RESULT, FLOW_DONE } FlowState;

typedef struct {
    FlowState state;
    Session rec;
    TimerState timer;
    double last_tick;
    int prev_distract_sec;
    int distracting;      // 1이면 경고 화면
    int warn_keyword;     // 이탈 키워드 번호, 없으면 -1
    int save_ok;   // 저장 성공 1, 실패 0
    int total_points; // 학습 종료 후 누적 포인트
} SessionFlow;

void flow_start(SessionFlow* f, const Session* plan);
void flow_update(SessionFlow* f);        // 매 프레임 호출
void flow_draw(const SessionFlow* f);    // BeginDrawing 안에서 호출

#endif