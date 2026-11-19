#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "result_ui.h"
#include "keyword.h"
#include "detect.h"

void result_ui(const char* subject, int goal_min,
               int study_sec, int distract_sec,
               int distract_count, int earn,
               int penalty, int bonus, int total_points) {

    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);

    int elapsed = study_sec + distract_sec;
    int net_points = earn - penalty + bonus;
    double focus_rate = 0;// 집중률 계산
    
    if (elapsed > 0)
        focus_rate = study_sec * 100.0 / elapsed;

    system("cls");

    printf("========================================\n");
    printf("           0:Tracker-RESULT\n");
    printf("========================================\n\n");

    printf("학습 과목 : %s\n", subject);
    printf("학습 목표 시간 : %d 분\n", goal_min);
    printf("목표 달성 여부 : %s\n\n",
        study_sec >= goal_min * 60 ? "달성" : "미달성");

    printf("--------------- 학습 결과 ---------------\n\n");

    printf("총 공부 시간 : %02d:%02d:%02d\n",
        elapsed / 3600, elapsed / 60 % 60,
        elapsed % 60);

    SetConsoleTextAttribute(h, 10);
    printf("순수 공부 시간 : %02d:%02d:%02d\n",
        study_sec / 3600, study_sec / 60 % 60,
        study_sec % 60);

    SetConsoleTextAttribute(h, 12);
    printf("학습 이탈 시간 : %02d:%02d:%02d\n",
        distract_sec / 3600, distract_sec / 60 % 60,
        distract_sec % 60);

    printf("학습 이탈 횟수 : %d 회\n", distract_count);

    SetConsoleTextAttribute(h, 10);
    printf("집중률 : %.1f%%\n\n", focus_rate);

    SetConsoleTextAttribute(h, 7);
    printf("----------- 키워드별 체류 시간 ----------\n\n");

    for (int i = 0; i < keyword_count(); i++)
    {
        int sec = get_keyword_sec(i);

        if (sec > 0)
            printf("%s : %02d:%02d:%02d\n",
                keyword_at(i),
                sec / 3600, sec / 60 % 60, sec % 60);
    }

    if (distract_sec == 0)
        printf("학습 이탈 기록이 없습니다.\n");

    printf("\n");
    printf("------------- POINT REPORT -------------\n\n");

    SetConsoleTextAttribute(h, 10);
    printf("획득 포인트 : +%d P\n", earn);

    SetConsoleTextAttribute(h, 12);
    printf("감점 포인트 : -%d P\n", penalty);

    SetConsoleTextAttribute(h, 14);
    printf("보너스 포인트 : +%d P\n", bonus);

    SetConsoleTextAttribute(h, 7);
    printf("----------------------------------------\n");

    SetConsoleTextAttribute(h, 14);
    printf("총 획득 포인트 : %+d P\n", net_points);
    printf("누적 포인트 : %d P\n\n", total_points);

    SetConsoleTextAttribute(h, 7);
    printf("========================================\n");

    fflush(stdout);
}