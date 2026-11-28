
#ifndef UI_THEME_H
#define UI_THEME_H

#include "raylib.h"

// 화면 크기
#define UI_WIDTH  800
#define UI_HEIGHT 480

// 메인 컬러
#define UI_PRIMARY      ((Color){152, 49, 81, 255})
#define UI_PRIMARY_DARK ((Color){112, 35, 60, 255})
#define UI_HOVER        ((Color){243, 229, 234, 255})

// 우왕이 컬러
#define UI_ORANGE ((Color){229, 138, 50, 255})
#define UI_CREAM  ((Color){248, 226, 181, 255})
#define UI_PINK   ((Color){243, 178, 166, 255})

// 기본 컬러
#define UI_BG      ((Color){250, 248, 246, 255})
#define UI_SURFACE ((Color){255, 255, 255, 255})
#define UI_TEXT    ((Color){48, 37, 41, 255})
#define UI_MUTED   ((Color){134, 119, 125, 255})
#define UI_BORDER  ((Color){230, 221, 224, 255})
#define UI_SUBTEXT ((Color){245, 222, 230, 255})
#define UI_WHITE   ((Color){255, 255, 255, 255})

// 상태별 컬러
#define UI_FOCUS    ((Color){84, 134, 107, 255})
#define UI_DISTRACT ((Color){197, 75, 80, 255})
#define UI_WARNING  ((Color){217, 154, 61, 255})

// 글자 크기
#define UI_FONT_TITLE    50
#define UI_FONT_SUBTITLE 35
#define UI_FONT_BODY     30
#define UI_FONT_SMALL    25
#define UI_FONT_TIMER    75

// 폰트 해상도 및 자간
#define UI_FONT_RESOLUTION 64
#define UI_FONT_SPACING    0.5f

// 여백
#define UI_MARGIN  24
#define UI_PADDING 16
#define UI_GAP     12

// 모서리 둥글기
#define UI_ROUNDNESS 0.2f

// 글꼴
#define UI_FONT_PATH "assets/fonts/Pretendard-Bold.otf"

#endif
