#include <stdio.h>
#include <stdlib.h>
#include "study_ui.h"
#include <windows.h>

int main() {
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    int menu;

    while (1) {
        system("cls");

        printf("========================================\n");
        printf("               0:Tracker\n");
        printf("========================================\n\n");
        printf("[1] 공부 시작\n\n");
        printf("[2] 학습 기록\n\n");
        printf("[3] Level\n\n");
        printf("[4] 설정\n\n");
        printf("[0] Exit\n\n");
        printf("----------------------------------------\n\n");
        printf("메뉴 선택>> ");

        if (scanf_s("%d", &menu) != 1) {
            while (getchar() != '\n');
            continue;
        }

        if (menu == 0) {
            printf("\n  Goodbye! Stay focused!\n");
            break;
        }

       if (menu == 1) {
            Study_ui("C Programming", 60, 1930, 190, 2);
        }
        else if (menu >= 2 && menu <= 4) {
                printf("\n  Menu %d selected.\n", menu);
            }
        else {
            printf("\n  Invalid selection. Try again.\n");
        }

        printf("\n  Press Enter to return...");
        while (getchar() != '\n');
        getchar();
    }

    return 0;
}
