#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include "study_ui.h"
#include "level_ui.h"
#include "warning.h"
#include "result_ui.h"


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
            Study_ui("C Programming", 60, 1930, 190, 2);// 학습 과목, 목표 시간(분), 순수 공부 시간(초), 이탈 시간(초), 이탈 횟수
            Sleep(3000);
            warning();
            while (1) {
                int key = _getch();
                if (key == 'q' || key == 'Q') {
                    result_ui("C Programming", 60,1930, 190, 2,320, 30, 0, 1291); // 학습 과목, 목표 시간, 순수 공부 시간, 이탈 시간, 이탈 횟수, 획득 포인트, 감점 포인트, 보너스 포인트, 누적 포인트
                    break;
                }
            }
        }

       else if (menu == 2) {
            printf("\n  Menu %d selected.\n", menu);
        }

       else if (menu == 3) {
            level_ui(1001);// 누적 포인트
        }

       else if (menu == 4) {
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
