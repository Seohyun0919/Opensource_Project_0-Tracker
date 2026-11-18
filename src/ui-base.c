#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "study_ui.h"
#include "level_ui.h"
#include "result_ui.h"
#include "timer.h"
#include "keyword.h"


int main() {
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    int menu;
    load_keywords();

    while (1) {
        system("cls");

        printf("========================================\n");
        printf("               0:Tracker\n");
        printf("========================================\n\n");
        printf("[1] 공부 시작\n\n");
        printf("[2] 학습 기록\n\n");
        printf("[3] Level\n\n");
        printf("[4] 키워드 설정\n\n");
        printf("[0] Exit\n\n");
        printf("----------------------------------------\n\n");
        printf("메뉴 선택>> ");

        int input_result = scanf_s("%d", &menu);
        int ch;
        if (input_result == EOF) {
            break;
        }
        while ((ch = getchar()) != '\n' && ch != EOF) {
        }
        if (input_result != 1) {
            continue;
        }

        if (menu == 0) {
            printf("\n  종료합니다 \n");
            break;
        }

       if (menu == 1) {
            Session rec = {0};

            system("cls");
            input_plan(&rec);
            if (feof(stdin) || ferror(stdin)) {
                break;
            }

            study_ui(&rec);
            result_ui(rec.subject, rec.goal_min, rec.study_sec,rec.distract_sec, rec.distract_count,320, 30, 0, 1291); // 포인트는 테스트 값
        }

       else if (menu == 2) {
            printf("\n  Menu %d selected.\n", menu);
        }

       else if (menu == 3) {
            level_ui(1001);// 누적 포인트
        }

       else if (menu == 4) {
            keyword();            
        }

       else {
            printf("\n  Invalid selection. Try again.\n");
        }

        printf("\n  Press Enter to return...");
        while ((ch = getchar()) != '\n' && ch != EOF) {
        }
    }

    return 0;
}
