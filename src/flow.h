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
} SessionFlow;

void flow_start(SessionFlow* f, const Session* plan);
void flow_update(SessionFlow* f);        // 매 프레임 호출
void flow_draw(const SessionFlow* f);    // BeginDrawing 안에서 호출

#endif