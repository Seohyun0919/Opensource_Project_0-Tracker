#include <stdio.h>
#include <windows.h>
#include "study_ui.h"

void Study_ui(const char* subject, int goal_min,
               int study_sec, int distract_sec,
               int distract_count) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    int elapsed = study_sec + distract_sec;
    int percent = 0;
    
    if (goal_min > 0)
    percent = study_sec * 100 / (goal_min * 60);

    if (percent > 100) percent = 100;

    // 커서를 화면 맨 위로 이동
    COORD pos = { 0, 0 };
    SetConsoleCursorPosition(h, pos);

    printf("========================================\n");
    printf("               0:Tracker\n");
    printf("========================================\n\n");

    printf("학습 과목 : %s\n\n", subject);

    printf("총 경과 시간 : %02d:%02d:%02d\n",
        elapsed / 3600, elapsed / 60 % 60, elapsed % 60);

    printf("순수 공부 시간 : %02d:%02d:%02d\n",
        study_sec / 3600, study_sec / 60 % 60,
        study_sec % 60);

    printf("학습 이탈 시간 : %02d:%02d:%02d\n",
        distract_sec / 3600, distract_sec / 60 % 60,
        distract_sec % 60);

    printf("학습 이탈 횟수 : %d 회\n", distract_count);
    printf("학습 목표 시간 : %d 분\n\n", goal_min);

    printf("목표 달성률 : %d%%\n  [", percent);

    SetConsoleTextAttribute(h, 10);
    for (int i = 0; i < 30; i++)
        printf(i < percent * 30 / 100 ? "#" : "-");
    SetConsoleTextAttribute(h, 7);

    printf("]\n\n");
    printf("----------------------------------------\n");
    printf("종료하려면 Q 키를 누르세요.\n");
    printf("========================================\n");
    fflush(stdout);
}
