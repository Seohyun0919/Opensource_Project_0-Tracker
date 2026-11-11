#include <stdio.h>
#include <windows.h>
#include "study_ui.h"

void showStudy(const char* subject, int goal_min,
               int study_sec, int distract_sec,
               int distract_count) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    int elapsed = study_sec + distract_sec;
    int percent = study_sec * 100 / (goal_min * 60);

    if (percent > 100) percent = 100;

    // 커서를 화면 맨 위로 이동
    COORD pos = { 0, 0 };
    SetConsoleCursorPosition(h, pos);

    printf("========================================\n");
    printf("               0:Tracker\n");
    printf("========================================\n\n");

    printf("  SUBJECT : %-25.25s\n\n", subject);

    printf("  Elapsed Time  : %02d:%02d:%02d\n",
        elapsed / 3600, elapsed / 60 % 60, elapsed % 60);

    printf("  Study Time    : %02d:%02d:%02d\n",
        study_sec / 3600, study_sec / 60 % 60,
        study_sec % 60);

    printf("  Distract Time : %02d:%02d:%02d\n",
        distract_sec / 3600, distract_sec / 60 % 60,
        distract_sec % 60);

    printf("  Distract Count: %d\n", distract_count);
    printf("  Goal Time     : %d min\n\n", goal_min);

    printf("  Goal Progress : %d%%\n  [", percent);

    SetConsoleTextAttribute(h, 10);
    for (int i = 0; i < 30; i++)
        printf(i < percent * 30 / 100 ? "#" : "-");
    SetConsoleTextAttribute(h, 7);

    printf("]\n\n");
    printf("----------------------------------------\n");
    printf("  Press Q to stop studying.\n");
    printf("========================================\n");
    fflush(stdout);
}
