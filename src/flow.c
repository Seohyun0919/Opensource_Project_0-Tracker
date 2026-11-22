#include "raylib.h"
#include "flow.h"

void flow_start(SessionFlow* f, const Session* plan)
{
    f->rec = *plan;
    f->rec.study_sec = 0;
    f->rec.distract_sec = 0;
    f->rec.distract_count = 0;
    f->rec.goal_achieved = 0;

    f->state = FLOW_STUDY;
    f->last_tick = GetTime();
    start_timer(&f->timer, f->last_tick);
}

void flow_update(SessionFlow* f)
{
    if (f->state == FLOW_STUDY) {
        double now = GetTime();
        int quit = IsKeyPressed(KEY_Q);

        // 1초가 지났을 때만 타이머·감지 갱신 (Sleep 사용 안 함)
        if (now - f->last_tick >= 1.0 || quit) {
            f->last_tick = now;
            update_timer(&f->timer, &f->rec, now);
        }
        if (quit) {
            f->state = FLOW_RESULT;
        }
    }
    else if (f->state == FLOW_RESULT) {
        if (IsKeyPressed(KEY_ENTER)) {
            f->state = FLOW_DONE;
        }
    }
}

void flow_draw(const SessionFlow* f)
{
    (void)f;
}