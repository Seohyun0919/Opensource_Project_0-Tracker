#include <stdio.h>
#include <windows.h>
#include "warning.h"

void showWarning(const char* window_title,
                 const char* keyword) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);

    SetConsoleTextAttribute(h, 12);

    printf("\n========================================\n");
    printf("               WARNING!\n");
    printf("========================================\n\n");
    printf("  학습 이탈이 감지되었습니다.\n\n");
    printf("  감지된 창 : %s\n", window_title);
    printf("  감지 키워드 : %s\n", keyword);
    printf("========================================\n");

    SetConsoleTextAttribute(h, 7);
}