// M8 메뉴·연결. 메인 메뉴를 보여 주고 각 모듈을 순서대로 부른다.
// 흐름: 입력(M1) -> 타이머 + 이탈 감지(M1·M2) -> 학습 중 화면(M7) -> 저장(M3)
//
// 기록 저장 모듈(record.c)이 develop에 합쳐지기 전에는 저장·조회 부분이 빠진 채로 빌드됨.
// record.c를 빌드에 넣은 뒤 컴파일할 때 /DHAVE_RECORD 를 붙이면 저장·조회가 켜짐.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "common.h"
#include "timer.h"
#include "detect.h"

#define MAX_LIST 100

// 초를 HH:MM:SS 형태로 출력
static void print_hms(int sec)
{
    printf("%02d:%02d:%02d", sec / 3600, sec / 60 % 60, sec % 60);
}

// 메뉴 번호를 한 줄로 입력받음. 숫자가 아니면 -1, 입력이 끊기면(EOF) 0(종료)
static int read_menu_choice(void)
{
    char line[32];
    char *end;
    long value;

    if (fgets(line, sizeof(line), stdin) == NULL) {
        return 0;
    }

    value = strtol(line, &end, 10);

    if (end == line) {
        return -1;
    }

    return (int)value;
}

static void wait_enter(void)
{
    char line[32];

    printf("\nEnter를 누르면 메뉴로 돌아갑니다...");
    fgets(line, sizeof(line), stdin);
}

static void print_banner(void)
{
    printf("========================================\n");
    printf("               0:Tracker\n");
    printf("========================================\n\n");
}

// 한 세션이 끝난 뒤 결과를 간단히 보여 줌 (결과 화면은 3주차 M7에서 대체)
static void print_session_summary(const Session *rec)
{
    system("cls");
    print_banner();

    printf("학습 과목 : %s\n\n", rec->subject);

    printf("순수 공부 시간 : ");
    print_hms(rec->study_sec);
    printf("\n학습 이탈 시간 : ");
    print_hms(rec->distract_sec);
    printf("\n학습 이탈 횟수 : %d 회\n", rec->distract_count);
    printf("학습 목표 시간 : %d 분\n\n", rec->goal_min);

    if (rec->goal_achieved) {
        printf("목표를 달성했습니다!\n");
    }
    else {
        printf("목표 달성에 실패했습니다.\n");
    }
}

// [1] 공부 시작: 입력 -> 타이머 -> 저장
static void menu_study(void)
{
    Session rec;

    memset(&rec, 0, sizeof(rec));

    system("cls");
    print_banner();

    input_plan(&rec);

#ifdef HAVE_RECORD
    rec.session_num = getNextSessionNum();
#endif

    run_timer(&rec);

    print_session_summary(&rec);

#ifdef HAVE_RECORD
    if (saveSession(&rec)) {
        printf("\n기록을 저장했습니다.\n");
    }
    else {
        printf("\n기록을 저장하지 못했습니다. (파일을 열 수 없음)\n");
    }
#else
    printf("\n(기록 저장 모듈이 아직 연결되지 않아 저장하지 않았습니다.)\n");
#endif

    wait_enter();
}

// [2] 기록 보기
static void menu_history(void)
{
#ifdef HAVE_RECORD
    static Session list[MAX_LIST];
    int n;
    int i;

    system("cls");
    print_banner();

    n = loadSessions(list, MAX_LIST);

    if (n > MAX_LIST) {
        n = MAX_LIST;
    }

    if (n <= 0) {
        printf("저장된 기록이 없습니다.\n");
    }

    for (i = 0; i < n; i++) {
        printf("#%d  %s  %s  공부 ", list[i].session_num, list[i].date, list[i].subject);
        print_hms(list[i].study_sec);
        printf("  이탈 %d회  %s\n", list[i].distract_count,
               list[i].goal_achieved ? "달성" : "미달성");
    }
#else
    printf("\n기록 저장 모듈이 아직 연결되지 않았습니다.\n");
#endif

    wait_enter();
}

// [3] 키워드 관리 (이탈 키워드 추가·삭제는 M2의 keyword_menu를 그대로 씀)
static void menu_keywords(void)
{
    system("cls");
    print_banner();

    keyword_menu();
}

int main(void)
{
    int choice;

    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // 이탈 키워드를 한 번 불러 둠 (타이머의 이탈 판정이 이 목록을 씀)
    load_keywords();

    while (1) {
        system("cls");
        print_banner();

        printf("[1] 공부 시작\n\n");
        printf("[2] 기록 보기\n\n");
        printf("[3] 키워드 관리\n\n");
        printf("[0] 종료\n\n");
        printf("----------------------------------------\n\n");
        printf("메뉴 선택>> ");

        choice = read_menu_choice();

        if (choice == 0) {
            printf("\n  Goodbye! Stay focused!\n");
            break;
        }

        else if (choice == 1) {
            menu_study();
        }

        else if (choice == 2) {
            menu_history();
        }

        else if (choice == 3) {
            menu_keywords();
        }

        else {
            printf("\n잘못된 선택입니다.\n");
            Sleep(800);
        }
    }

    return 0;
}
