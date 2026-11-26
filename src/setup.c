// M8 학습 설정 화면: 과목 선택 → (새 과목 입력) → 목표 시간 → 시작
// windows.h를 include하지 않음 (raylib.h와 이름이 겹침)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "setup.h"

static void today(char date[11])
{
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    strftime(date, 11, "%Y-%m-%d", t);
}

void setup_init(SetupState* s)
{
    memset(s, 0, sizeof(*s));
    today(s->date);
    s->subject_count = loadSubjects(s->subjects, SETUP_MAX_SUBJECTS);
    s->chosen = -1;
    s->step = SETUP_PICK_SUBJECT;
}

//입력 문자열 검사 함수 추가
// 한글이 3바이트라 strlen으로는 20자 제한을 못 잼. 이어붙는 바이트(10xxxxxx)는 세지 않음
static int utf8_len(const char* str)
{
    int n = 0;
    for (; *str; str++) {
        if (((unsigned char)*str & 0xC0) != 0x80) n++;
    }
    return n;
}

// fgets가 남기는 개행과 앞뒤 공백을 지움
static void trim(char* str)
{
    char* start = str;
    int len;
    while (*start == ' ' || *start == '\t') start++;
    memmove(str, start, strlen(start) + 1);
    len = (int)strlen(str);
    while (len > 0 && (str[len - 1] == ' ' || str[len - 1] == '\t'
                       || str[len - 1] == '\n' || str[len - 1] == '\r')) {
        str[--len] = '\0';
    }
}

static void set_message(SetupState* s, const char* msg)
{
    snprintf(s->message, sizeof(s->message), "%s", msg);
}

static void go_step(SetupState* s, SetupStep step)
{
    s->step = step;
    s->message[0] = '\0';
}

static void submit_pick(SetupState* s, const char* text)
{
    char* end;
    long num;

    if (text[0] == '\0') {
        set_message(s, "번호를 입력해 주세요.");
        return;
    }
    num = strtol(text, &end, 10);
    // "2a"를 2번으로 읽으면 오타가 그대로 통과함
    if (*end != '\0' || num < 0 || num > s->subject_count) {
        set_message(s, "목록에 없는 번호입니다. 다시 입력해 주세요.");
        return;
    }
    if (num == 0) {
        go_step(s, SETUP_NEW_SUBJECT);
    } else {
        s->chosen = (int)num - 1;
        go_step(s, SETUP_GOAL);
    }
}

int setup_submit(SetupState* s, const char* line, Session* plan)
{
    char text[256];

    snprintf(text, sizeof(text), "%s", line);
    trim(text);
    s->message[0] = '\0';

    switch (s->step) {
    case SETUP_PICK_SUBJECT:  submit_pick(s, text); break;
    case SETUP_NEW_SUBJECT:   break;
    case SETUP_GOAL:          break;
    }
    return 0;
}