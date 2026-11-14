#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "timer.h"

#define MIN_GOAL_MIN 1
#define MAX_GOAL_MIN 600
#define LINE_BUF_SIZE 256

// UTF-8 한글을 고려하여 글자 수를 계산하는 함수
int count_chars(char str[])
{
    int count = 0;
    int i;

    for (i = 0; str[i] != '\0'; i++) {
        // 한글은 3바이트이므로 첫 번째 바이트만 글자 수로 계산
        if ((str[i] & 0xC0) != 0x80) {
            count++;
        }
    }

    return count;
}

// 문자열을 한 줄 입력받는 함수
int read_line(char buf[], int size)
{
    if (fgets(buf, size, stdin) == NULL) {
        return 0;
    }

    // 입력받은 문자열의 줄바꿈 문자 제거
    buf[strcspn(buf, "\n")] = '\0';

    return 1;
}

// 이름이나 과목명을 입력받는 함수
void input_text(char prompt[], char out[], int max_chars)
{
    char line[LINE_BUF_SIZE];
    int len;

    while (1) {
        printf("%s (1~%d자): ", prompt, max_chars);

        if (read_line(line, LINE_BUF_SIZE) == 0) {
            out[0] = '\0';
            return;
        }

        len = count_chars(line);

        // 빈 문자열 검사
        if (len == 0) {
            printf("빈칸은 입력할 수 없습니다.\n");
        }

        // 최대 글자 수 검사
        else if (len > max_chars) {
            printf("%d자를 넘을 수 없습니다.\n", max_chars);
        }

        // 배열에 저장할 수 있는 바이트 크기 검사
        else if (strlen(line) > max_chars * 3) {
            printf("입력한 문자열이 너무 깁니다.\n");
        }

        // 조건을 모두 만족하면 문자열 저장
        else {
            strcpy(out, line);
            break;
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

    while (1) {
        printf("목표 시간 (%d~%d분): ", MIN_GOAL_MIN, MAX_GOAL_MIN);

        // 입력이 끊기면 최솟값을 돌려주고 종료
        if (read_line(line, LINE_BUF_SIZE) == 0) {
            return MIN_GOAL_MIN;
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

        // 조건을 모두 만족하면 값을 돌려줌
        else {
            return (int)value;
        }
    }
}

// 오늘 날짜를 YYYY-MM-DD 형태로 만드는 함수
void get_today(char date[])
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