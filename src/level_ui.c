
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "level_ui.h"

void level_ui(int total_points) {
    const char* grades[] = {
        "F", "D", "C", "B", "A", "A+"
    };
    int points[] = {
        0, 1000, 5000, 10000, 50000, 100000
    };
    int level = 0, progress = 100;
    int current_points, next_points;

    SetConsoleOutputCP(CP_UTF8);

    if (total_points < 0) total_points = 0;

    for (int i = 1; i < 6; i++)
        if (total_points >= points[i]) level = i;

    current_points = total_points - points[level];

    if (level < 5) {
        next_points = points[level + 1] - points[level];
        progress = current_points * 100 / next_points;
    }

    system("cls");

    printf("========================================\n");
    printf("           0:Tracker - level\n");
    printf("========================================\n\n");

    printf("  현재 등급 : %s\n\n", grades[level]);
    printf("  현재 포인트 : %d P\n\n", current_points);
    printf("  진행도  %d%%\n", progress);

    if (level < 5)
        printf("  ( %dp / %dp )\n",
               current_points, next_points);
    else
        printf("  ( MAX )\n");

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    printf("  [");
    for (int i = 0; i < 30; i++) {
        if (i < progress * 30 / 100) {
            SetConsoleTextAttribute(hConsole, 10);
            printf("#");
        }
        else {
            SetConsoleTextAttribute(hConsole, 7);
            printf("-");
        }
    }

    SetConsoleTextAttribute(hConsole, 7);
    printf("]\n");
    printf("========================================\n");
}
