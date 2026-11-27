#include "raylib.h"
#include "flow.h"
#include "detect.h"
#include "point.h"
#include "gui.h"
#include "ui_color.h"

static const Rectangle stopRect = {36, 418, 728, 49};
static const Rectangle backRect = {410, 420, 354, 52};
static const Rectangle detailRect = {36, 420, 354, 52};

// 버튼 클릭 확인
static int clicked(Rectangle rect) {
    return CheckCollisionPointRec(
        GetMousePosition(), rect
    ) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

// 버튼 출력
static void draw_button(Rectangle rect, const char* text) {
    int hover = CheckCollisionPointRec(
        GetMousePosition(), rect
    );

    DrawRectangleRounded(
        rect, 0.2f, 8,
        hover ? UI_PRIMARY_DARK : UI_PRIMARY
    );

    DrawText(
        text,
        rect.x + (rect.width - MeasureText(text, 20)) / 2,
        rect.y + (rect.height - 20) / 2,
        20, WHITE
    );
}

// 학습 시작
void flow_start(SessionFlow* f, const Session* plan) {
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
    f->show_detail = 0;
    f->total_points = 0;

    f->state = FLOW_STUDY;
    f->last_tick = GetTime();

    start_timer(&f->timer, f->last_tick);
}

// 학습 상태 갱신
void flow_update(SessionFlow* f) {

    if (f->state == FLOW_STUDY) {
        double now = GetTime();

        int quit = IsKeyPressed(KEY_Q)
                   || clicked(stopRect);

        // 1초마다 타이머 및 이탈 감지 갱신
        if (now - f->last_tick >= 1.0 || quit) {
            f->last_tick = now;

            update_timer(&f->timer, &f->rec, now);

            f->distracting =
                f->rec.distract_sec > f->prev_distract_sec;

            f->prev_distract_sec = f->rec.distract_sec;

            f->warn_keyword = f->distracting
                ? detect_current_window() : -1;
        }

        // 공부 종료
        if (quit) { 
            f->rec.goal_achieved =        
            f->rec.study_sec >= f->rec.goal_min * 60;    
            f->save_ok = saveSession(&f->rec); 
            // 포인트 계산 및 저장    
            f->total_points = point(        
                f->rec.study_sec,        
                f->rec.distract_sec    
            );    
            f->state = FLOW_RESULT;
        }
    }
    else if (f->state == FLOW_RESULT) {    
        if (clicked(detailRect)) {        
            f->show_detail = !f->show_detail;    
        }    
        if (clicked(backRect) || IsKeyPressed(KEY_ENTER)) {        
            f->state = FLOW_DONE;    
        }
    }
}

// 학습 화면 출력
void flow_draw(const SessionFlow* f) {

    if (f->state == FLOW_STUDY) {

        if (f->distracting) {
            draw_warning_screen(f->warn_keyword);
        }
        else {
            draw_study_screen(
                f->rec.subject,
                f->rec.goal_min,
                f->rec.study_sec,
                f->rec.distract_sec,
                f->rec.distract_count
            );
        }

        draw_button(stopRect, "Stop Study");
    }

    else if (f->state == FLOW_RESULT) {

        // 상세 기록 화면
        if (f->show_detail) {
            draw_detail_screen(
                f->rec.subject,
                f->rec.goal_min,
                f->rec.study_sec,
                f->rec.distract_sec,
                f->rec.distract_count,
                earn(f->rec.study_sec),
                penalty(f->rec.distract_sec),
                0,
                f->total_points
            );
        }

        // 결과 요약 화면
        else {
            draw_result_screen(
                f->rec.subject,
                f->rec.goal_min,
                f->rec.study_sec,
                f->rec.distract_sec,
                f->rec.distract_count,
                earn(f->rec.study_sec),
                penalty(f->rec.distract_sec),
                0,
                f->total_points
            );
        }

        if (!f->save_ok)
            draw_save_error();
    }
}