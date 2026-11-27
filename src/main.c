
#include "raylib.h"
#include "ui_color.h"
#include "gui.h"
#include "flow.h"
#include "keyword.h"

static Font font;
static Texture2D uwang;

// 메뉴 이름
static const char* menus[] = {
    "공부 시작",
    "학습 기록",
    "Level",
    "키워드 설정",
    "Exit"
};

// 가운데 정렬 텍스트
static void center_text(const char* text, int y,
                        int size, Color color) {

    Vector2 t = MeasureTextEx(
        font, text, size, UI_FONT_SPACING
    );

    DrawTextEx(
        font, text,
        (Vector2){(UI_WIDTH - t.x) / 2, y},
        size, UI_FONT_SPACING, color
    );
}

// 공통 버튼
static int button(Rectangle r, const char* text) {

    int hover = CheckCollisionPointRec(
        GetMousePosition(), r
    );

    DrawRectangleRounded(
        r, UI_ROUNDNESS, 10,
        hover ? UI_HOVER : UI_SURFACE
    );

    DrawTextEx(
        font, text,
        (Vector2){
            r.x + 20,
            r.y + (r.height - UI_FONT_BODY) / 2
        },
        UI_FONT_BODY,
        UI_FONT_SPACING,
        UI_TEXT
    );

    return hover &&
           IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

// 상단 헤더
static void header(const char* title) {

    DrawRectangleRounded(
        (Rectangle){24, 24, 752, 88},
        0.25f, 10, UI_PRIMARY
    );

    center_text(
        title, 34,
        UI_FONT_TITLE, UI_WHITE
    );

    center_text(
        "Your personal study tracker",
        80, UI_FONT_SMALL, UI_SUBTEXT
    );
}

// 우왕이 사진 출력
static void draw_uwang(void) {

    if (uwang.id == 0)
        return;

    // 사진 원본 비율 유지
    float sx = 300.0f / uwang.width;
    float sy = 250.0f / uwang.height;
    float scale = sx < sy ? sx : sy;

    DrawTextureEx(
        uwang,
        (Vector2){
            585 - uwang.width * scale / 2,
            250 - uwang.height * scale / 2
        },
        0.0f, scale, WHITE
    );
}

// 폰트 초기화
static void load_font(void) {

    int chars[12000];
    int n = 0;

    // 영문, 숫자, 특수문자
    for (int i = 32; i <= 126; i++)
        chars[n++] = i;

    // 한글 자모
    for (int i = 0x3131; i <= 0x318E; i++)
        chars[n++] = i;

    // 한글 완성형
    for (int i = 0xAC00; i <= 0xD7A3; i++)
        chars[n++] = i;

    font = LoadFontEx(
        UI_FONT_PATH,
        32,
        chars, n
    );

    SetTextureFilter(
        font.texture,
        TEXTURE_FILTER_BILINEAR
    );
}

// 메인 함수
int main(void) {

    // GUI 창 생성
    SetConfigFlags(FLAG_MSAA_4X_HINT);

    InitWindow(
        UI_WIDTH, UI_HEIGHT,
        "0:Tracker"
    );

    Image icon = LoadImage("assets/icon2.png");
    SetWindowIcon(icon);
    UnloadImage(icon);

    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    // 폰트 불러오기
    load_font();
    gui_init(font);
    load_keywords();

    // 우왕이 사진 불러오기
    uwang = LoadTexture("assets/uwang.png");

    if (uwang.id != 0) {
        SetTextureFilter(
            uwang,
            TEXTURE_FILTER_BILINEAR
        );
    }

    int screen = 0;
    int running = 1;

    SetupState setup = {0};
    Session plan = {0};
    SessionFlow flow = {0};

    // 메인 반복문
    while (!WindowShouldClose() && running) {

        BeginDrawing();
        ClearBackground(UI_BG);

        // 메인 메뉴
        if (screen == 0) {

            header("0:Tracker");

            draw_uwang();

            // 메뉴 버튼 4개
            for (int i = 0; i < 4; i++) {

                if (button(
                    (Rectangle){
                        36, 130 + i * 60,
                        370, 54
                    },
                    menus[i]
                )) {
                    screen = i + 1;

                    if (screen == 1)    
                        setup_init(&setup);
                }
            }

            // 구분선
            DrawLine(
                36, 380,
                764, 380,
                UI_BORDER
            );

            // 종료 버튼
            if (button(
                (Rectangle){36, 397, 728, 52},
                menus[4]
            )) {
                running = 0;
            }
        }

        // 각 메뉴의 임시 화면
        // 학습 계획 입력 화면
else if (screen == 1) {

    setup_draw(&setup);

    int result = setup_update(&setup, &plan);

    if (result == 1) {
        flow_start(&flow, &plan);
        screen = 5;
    }

    else if (result == -1) {
        screen = 0;
    }
}

// 학습 진행 및 결과 화면
else if (screen == 5) {

    flow_update(&flow);

    if (flow.state == FLOW_DONE) {
        screen = 0;
    }

    else {
        flow_draw(&flow);
    }
}

// 나머지 메뉴 임시 화면
else {

    header(menus[screen - 1]);

    center_text(
        "준비 중",
        220,
        UI_FONT_SUBTITLE,
        UI_MUTED
    );

    if (button(
        (Rectangle){36, 397, 728, 52},
        "메인으로"
    ) || IsKeyPressed(KEY_ESCAPE)) {

        screen = 0;
    }
}

        EndDrawing();
    }

    // 이미지 메모리 해제
    if (uwang.id != 0)
        UnloadTexture(uwang);

    // 폰트 메모리 해제
    if (font.texture.id != GetFontDefault().texture.id)
        UnloadFont(font);

    CloseWindow();

    return 0;
}
