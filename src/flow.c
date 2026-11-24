#include "raylib.h"
#include "flow.h"
#include "detect.h"

void flow_start(SessionFlow* f, const Session* plan)
{
    f->rec = *plan;
    f->rec.study_sec = 0;
    f->rec.distract_sec = 0;
    f->rec.distract_count = 0;
    f->rec.goal_achieved = 0;
    f->prev_distract_sec = 0;
    f->distracting = 0;
    f->warn_keyword = -1;
    f->rec.session_num = getNextSessionNum();
    f->save_ok = 0;
    
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

            f->distracting = (f->rec.distract_sec > f->prev_distract_sec);
            f->prev_distract_sec = f->rec.distract_sec;
            f->warn_keyword = f->distracting ? detect_current_window() : -1;
        }
        if (quit) {
            f->save_ok = saveSession(&f->rec);
            f->state = FLOW_RESULT;
        }
    }
    else if (f->state == FLOW_RESULT) {
        if (IsKeyPressed(KEY_ENTER)) {
            f->state = FLOW_DONE;
        }
    }
}

//이후 feature/ui-raylib 함수 이름으로 교체
void flow_draw(const SessionFlow* f)
{
    if (f->state == FLOW_STUDY) {
        if (f->distracting)
            draw_warning_screen(f->warn_keyword);
        
        else
            draw_study_screen(f->rec.subject, f->rec.goal_min,
                              f->rec.study_sec, f->rec.distract_sec,
                              f->rec.distract_count);
    }

    // flow_draw에 결과 화면 추가 (포인트는 levelup 전까지 테스트 값)
    else if (f->state == FLOW_RESULT) {
        draw_result_screen(f->rec.subject, f->rec.goal_min,
                           f->rec.study_sec, f->rec.distract_sec,
                           f->rec.distract_count, 320, 30, 0, 1291);
        if (!f->save_ok)
            draw_save_error();   // 이후 feature/ui-raylib 함수 이름으로 교체
    }
}
