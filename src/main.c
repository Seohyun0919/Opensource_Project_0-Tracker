
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
        64,
        chars, n
    );

    SetTextureFilter(
        font.texture,
        TEXTURE_FILTER_BILINEAR
    );
}

static int draw_main_menu(Font font, Texture2D uwang) {

    // 왼쪽 캐릭터 배경
    DrawRectangleRounded(
        (Rectangle){26, 26, 345, 428},
        0.14f, 20, UI_HOVER
    );

    // 0:Tracker 제목
    Vector2 t = MeasureTextEx(font, "0:Tracker", 52, 1);

    DrawTextEx(
        font, "0:Tracker",
        (Vector2){198.5f - t.x / 2, 48},
        52, 1, UI_PRIMARY
    );

    // 우왕이 이미지
    if (uwang.id != 0) {

        float sx = 310.0f / uwang.width;
        float sy = 310.0f / uwang.height;

        float scale = sx < sy ? sx : sy;

        float w = uwang.width * scale;
        float h = uwang.height * scale;

        DrawTextureEx(
            uwang,
            (Vector2){
                180 - w / 2,
                280 - h / 2
            },
            0, scale, WHITE
        );
    }

    // 메뉴 버튼 영역
    Rectangle rects[] = {
        {395, 44, 379, 100},
        {395, 160, 379, 62},
        {395, 235, 379, 62},
        {395, 310, 379, 62},
        {395, 395, 379, 50}
    };

    const char* labels[] = {
        "공부 시작",
        "학습 기록",
        "Level",
        "키워드 설정",
        "Exit"
    };

    int selected = 0;

    for (int i = 0; i < 5; i++) {

        Rectangle r = rects[i];

        int hover = CheckCollisionPointRec(
            GetMousePosition(), r
        );

        Color bg = i == 0
            ? (hover ? UI_PRIMARY_DARK : UI_PRIMARY)
            : (hover ? UI_HOVER : UI_SURFACE);

        Color fg = i == 0 ? UI_WHITE : UI_TEXT;

        // 버튼 배경
        DrawRectangleRounded(r, 0.25f, 20, bg);

        // 글자 크기
        float size = i == 0 ? 38 : 30;

        if (i == 4)
            size = 25;

        t = MeasureTextEx(font, labels[i], size, 1);

        // 글자 위치
        float x = i == 0 || i == 4
            ? r.x + (r.width - t.x) / 2
            : 470;

        float y = r.y + (r.height - t.y) / 2;

        DrawTextEx(
            font, labels[i],
            (Vector2){x, y},
            size, 1, fg
        );

        // 공부 시작 아이콘
        if (i == 0) {

            DrawTriangle(
                (Vector2){430, 77},
                (Vector2){430, 111},
                (Vector2){459, 94},
                UI_WHITE
            );
        }

        // 학습 기록 아이콘
        else if (i == 1) {

            DrawRectangleLinesEx(
                (Rectangle){422, 178, 24, 27},
                2.5f, UI_PRIMARY
            );

            DrawLine(428, 186, 440, 186, UI_PRIMARY);
            DrawLine(428, 193, 440, 193, UI_PRIMARY);
            DrawLine(428, 200, 437, 200, UI_PRIMARY);
        }

        // Level 아이콘
        else if (i == 2) {

            DrawPoly(
                (Vector2){434, 266},
                6, 17, 0, UI_PRIMARY
            );

            DrawLineEx(
                (Vector2){427, 269},
                (Vector2){434, 260},
                3, WHITE
            );

            DrawLineEx(
                (Vector2){434, 260},
                (Vector2){441, 269},
                3, WHITE
            );
        }

        // 키워드 설정 아이콘
        else if (i == 3) {

            DrawPoly(
                (Vector2){434, 341},
                8, 15, 0, UI_PRIMARY
            );

            DrawCircle(434, 341, 8, bg);
            DrawCircle(434, 341, 4, UI_PRIMARY);
        }

        // 오른쪽 화살표
        if (i >= 1 && i <= 3) {

            float cy = r.y + r.height / 2;

            DrawLineEx(
                (Vector2){738, cy - 7},
                (Vector2){745, cy},
                2.5f, UI_PRIMARY
            );

            DrawLineEx(
                (Vector2){745, cy},
                (Vector2){738, cy + 7},
                2.5f, UI_PRIMARY
            );
        }

        // 버튼 클릭 처리
        if (hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            selected = i + 1;
    }

    return selected;
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

        gui_begin(screen);
        ClearBackground(UI_BG);

        // 메인 메뉴
        if (screen == 0) {    
            int selected = draw_main_menu(font, uwang);    
            
            if (selected == 5) {        
                running = 0;    
            }    
            else if (selected > 0) {        
                screen = selected;        
                if (screen == 1)            
                setup_init(&setup);    

                else if (screen == 2)  
                history_init();
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
        // 학습 기록 화면
        else if (screen == 2) {  
            if (history_update() == -1) {    
                history_close();      
                screen = 0;   
            }
  
            else {    
                history_draw();  
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
 
        gui_end();  
    }

    
    // 이미지 메모리 해제   
    if (uwang.id != 0)
        UnloadTexture(uwang);

    // 폰트 메모리 해제
    if (font.texture.id != GetFontDefault().texture.id)
        UnloadFont(font);    
        
        history_close();
        gui_close();
        CloseWindow();

    return 0;
}
