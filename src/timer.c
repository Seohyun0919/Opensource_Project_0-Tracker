// M1 계획·타이머. 이름·과목·목표 시간 입력 검사와 Enter/Q 타이머.
// 결과(study_sec, goal_achieved 등)는 Session에 채워 둠. 저장은 M3 몫.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <conio.h>
#include <windows.h>
#include "timer.h"


#define MIN_GOAL_MIN 1
#define MAX_GOAL_MIN 600
#define LINE_BUF_SIZE 256

// 문자열의 글자 수를 계산하는 함수
static int count_chars(const char *str)
{
    int count = 0;
    int i;

    for (i = 0; str[i] != '\0'; i++) {
        // UTF-8 이어지는 바이트는 글자 수에서 제외
        if (((unsigned char)str[i] & 0xC0) != 0x80) {
            count++;
        }
    }

    return count;
}

// 한 줄을 입력받는 함수
// 성공: 1, 입력 실패: 0, 입력 길이 초과: -1
static int read_line(char buf[], int size)
{
    int ch;

    if (fgets(buf, size, stdin) == NULL) {
        return 0;
    }

    // 입력한 문자열에 줄바꿈이 있는지 확인
    if (strchr(buf, '\n') == NULL && !feof(stdin)) {

        // 버퍼에 남아 있는 입력 내용 제거
        while ((ch = getchar()) != '\n' && ch != EOF) {
        }

        return -1;
    }

    // 줄바꿈 문자 제거
    buf[strcspn(buf, "\r\n")] = '\0';

    return 1;
}

// 이름이나 과목명을 입력받는 함수
static void input_text(const char *prompt, char out[], int max_chars)
{
    char line[LINE_BUF_SIZE];
    int len;
    int result;

    while (1) {
        printf("%s (1~%d자): ", prompt, max_chars);

        result = read_line(line, LINE_BUF_SIZE);

        // 입력 실패 시 함수 종료
        if (result == 0) {
            out[0] = '\0';
            return;
        }

        // 입력 길이가 너무 긴 경우
        if (result == -1) {
            printf("입력이 너무 깁니다. 다시 입력하세요.\n");
            continue;
        }

        // 입력한 글자 수 계산
        len = count_chars(line);

        // 빈칸 검사
        if (len == 0) {
            printf("빈칸은 입력할 수 없습니다.\n");
        }

        // 최대 글자 수 검사
        else if (len > max_chars) {
            printf("%d자를 넘을 수 없습니다.\n", max_chars);
        }

        // 배열에 저장할 수 있는 바이트 크기 검사
        else if (strlen(line) > (size_t)max_chars * 3) {
            printf("지원하지 않는 문자가 있습니다.\n");
        }

        // 모든 조건을 만족하면 문자열 저장
        else {
            strcpy(out, line);
            return;
        }
    }
}

// 사용자 이름 입력
void input_user_name(char user_name[])
{
    input_text("이름", user_name, MAX_NAME_LEN);
}

// 과목명 입력
void input_subject(char subject[])
{
    input_text("과목명", subject, MAX_SUBJECT_LEN);
}

// 목표 시간(분)을 입력받는 함수
int input_goal_min(void)
{
    char line[LINE_BUF_SIZE];
    char *end;
    long value;
    int result;

    while (1) {
        printf("목표 시간 (%d~%d분): ", MIN_GOAL_MIN, MAX_GOAL_MIN);

        result = read_line(line, LINE_BUF_SIZE);

        // 입력 실패 시 최솟값을 돌려주고 종료
        if (result == 0) {
            return MIN_GOAL_MIN;
        }

        // 입력 길이가 너무 긴 경우
        if (result == -1) {
            printf("입력이 너무 깁니다. 다시 입력하세요.\n");
            continue;
        }

        value = strtol(line, &end, 10);

        // 숫자가 아닌 입력 검사 (12abc처럼 뒤에 글자가 남은 경우도 포함)
        if (end == line || *end != '\0') {
            printf("숫자만 입력하세요.\n");
        }

        // 범위 검사
        else if (value < MIN_GOAL_MIN || value > MAX_GOAL_MIN) {
            printf("%d~%d분 사이로 입력하세요.\n", MIN_GOAL_MIN, MAX_GOAL_MIN);
        }

        // 모든 조건을 만족하면 값을 돌려줌
        else {
            return (int)value;
        }
    }
}

// 오늘 날짜를 YYYY-MM-DD 형태로 만드는 함수
static void get_today(char date[])
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    strftime(date, 11, "%Y-%m-%d", t);
}

// 이름, 과목, 목표 시간, 날짜를 입력받아 세션에 저장하는 함수
void input_plan(Session *rec)
{
    input_user_name(rec->user_name);
    input_subject(rec->subject);
    rec->goal_min = input_goal_min();
    get_today(rec->date);

    // M3 함수가 합쳐지기 전까지 세션 번호는 0으로 둠
    rec->session_num = 0;
}

// 타이머를 실행하는 함수 (Enter로 시작, Q로 종료)
void run_timer(Session *rec)
{
    int ch;
    int has_key;
    int elapsed = 0;
    int printed_sec = -1;
    time_t start;
    time_t now;
    int goal_sec;

    printf("\nEnter를 누르면 시작합니다. (종료: Q)\n");

    // Enter 키를 누를 때까지 대기
    ch = _getch();
    while (ch != '\r') {
        ch = _getch();
    }

    start = time(NULL);

    while (1) {
        // 키가 눌렸는지 확인 (눌리지 않았으면 기다리지 않고 넘어감)
        has_key = _kbhit();

        if (has_key != 0) {
            ch = _getch();

            // Q 키를 누르면 타이머 종료
            if (ch == 'q' || ch == 'Q') {
                break;
            }
        }

        // 시작한 뒤 지난 시간(초) 계산
        now = time(NULL);
        elapsed = (int)difftime(now, start);

        // 초가 바뀐 경우에만 화면 갱신
        if (elapsed != printed_sec) {
            printf("\r경과 시간: %02d:%02d   ", elapsed / 60, elapsed % 60);
            fflush(stdout);
            printed_sec = elapsed;
        }

        // CPU 사용을 줄이기 위해 0.1초 대기
        Sleep(100);
    }

    // 목표 시간(분)을 초로 바꿔서 비교
    goal_sec = rec->goal_min * 60;

    // 공부한 시간은 경과 시간 전체로 저장 (이탈 시간 처리는 M2 연결 후 정함)
    rec->study_sec = elapsed;
    rec->distract_sec = 0;
    rec->distract_count = 0;

    // 목표 시간과 같아도 달성으로 봄
    if (rec->study_sec >= goal_sec) {
        rec->goal_achieved = 1;
    }

    // 목표 시간보다 적게 공부한 경우
    else {
        rec->goal_achieved = 0;
    }

    printf("\n\n종료되었습니다.\n");
}