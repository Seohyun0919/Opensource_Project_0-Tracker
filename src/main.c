// M8 메뉴·연결: raylib 창 하나에서 설정 → 학습(타이머·이탈 감지·경고) → 저장 → 결과 흐름을 돌린다.
// 주의: 이 파일은 windows.h를 include하지 않는다 (raylib.h와 이름이 겹침).
#include "raylib.h"
#include "common.h"
#include "flow.h"
#include "keyword.h"

typedef enum { SCREEN_SETUP, SCREEN_FLOW } Screen;

int main(void)
{
    InitWindow(800, 480, "0:Tracker");
    SetExitKey(KEY_NULL);          // ESC로 창이 닫히지 않게 함
    SetTargetFPS(30);
    ui_init();
    load_keywords();

    Screen screen = SCREEN_SETUP;
    SetupState setup;
    SessionFlow flow;
    Session plan;
    setup_init(&setup);

    while (!WindowShouldClose()) {
        if (screen == SCREEN_SETUP) {
            if (setup_update(&setup, &plan)) {
                flow_start(&flow, &plan);
                screen = SCREEN_FLOW;
            }
        } else {
            flow_update(&flow);
            if (flow.state == FLOW_DONE) {
                setup_init(&setup);
                screen = SCREEN_SETUP;
            }
        }

        BeginDrawing();
        if (screen == SCREEN_SETUP) setup_draw(&setup);
        else flow_draw(&flow);
        EndDrawing();
    }

    ui_close();
    CloseWindow();
    return 0;
}
