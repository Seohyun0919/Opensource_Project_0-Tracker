// M8 학습 설정 화면: 과목 선택 → (새 과목 입력) → 목표 시간 → 시작
// windows.h를 include하지 않음 (raylib.h와 이름이 겹침)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "setup.h"

#define GOAL_MIN_LIMIT  1
#define GOAL_MAX_LIMIT  600

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

// 과목 번호 선택 처리 추가
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

// 새 과목 입력과 저장 추가
static void submit_new_subject(SetupState* s, const char* text)
{
    int i;

    // 글자 수와 별개로 바이트도 확인함. 4바이트 글자가 섞이면 20자여도 배열을 넘음
    if (text[0] == '\0' || utf8_len(text) > MAX_SUBJECT_LEN
        || strlen(text) >= SETUP_SUBJECT_SIZE) {
        set_message(s, "과목명은 1~20자로 입력해 주세요.");
        return;
    }
    // 같은 이름을 또 저장하면 목록에 중복이 쌓임. 있으면 그 과목을 고른 것으로 처리
    for (i = 0; i < s->subject_count; i++) {
        if (strcmp(s->subjects[i], text) == 0) {
            s->chosen = i;
            go_step(s, SETUP_GOAL);
            return;
        }
    }
    if (s->subject_count >= SETUP_MAX_SUBJECTS) {
        set_message(s, "과목은 30개까지 저장할 수 있습니다.");
        return;
    }
    snprintf(s->subjects[s->subject_count], SETUP_SUBJECT_SIZE, "%s", text);
    s->chosen = s->subject_count;
    s->subject_count++;
    go_step(s, SETUP_GOAL);
    // 저장에 실패해도 이번 학습은 이어갈 수 있게 막지 않고 안내만 함
    if (!saveSubjects(s->subjects, s->subject_count)) {
        set_message(s, "과목 목록 파일을 저장하지 못했습니다.");
    }
}

//목표 시간 입력과 범위 검사 추가
static int submit_goal(SetupState* s, const char* text, Session* plan)
{
    char* end;
    long goal = strtol(text, &end, 10);

    if (text[0] == '\0' || *end != '\0'
        || goal < GOAL_MIN_LIMIT || goal > GOAL_MAX_LIMIT) {
        set_message(s, "목표 시간은 1~600분 사이로 입력해 주세요.");
        return 0;
    }
    memset(plan, 0, sizeof(*plan));
    snprintf(plan->subject, sizeof(plan->subject), "%s", s->subjects[s->chosen]);
    snprintf(plan->date, sizeof(plan->date), "%s", s->date);
    plan->goal_min = (int)goal;
    return 1;
}

int setup_submit(SetupState* s, const char* line, Session* plan)
{
    char text[256];

    snprintf(text, sizeof(text), "%s", line);
    trim(text);
    s->message[0] = '\0';

    switch (s->step) {
    case SETUP_PICK_SUBJECT:  submit_pick(s, text); break;
    case SETUP_NEW_SUBJECT:   submit_new_subject(s, text); break;
    case SETUP_GOAL:          return submit_goal(s, text, plan);
    }
    return 0;
}

void setup_print(const SetupState* s)
{
    int i;

    system("cls");
    printf("========================================\n");
    printf("          0:Tracker | 학습 설정\n");
    printf("========================================\n\n");
    printf("날짜 : %s\n\n", s->date);

    if (s->step == SETUP_PICK_SUBJECT) {
        printf("[0] 새로운 과목 추가\n");
        for (i = 0; i < s->subject_count; i++) {
            printf("[%d] %s\n", i + 1, s->subjects[i]);
        }
        printf("\n");
    } else if (s->step == SETUP_GOAL) {
        printf("과목 : %s\n\n", s->subjects[s->chosen]);
    }

    if (s->message[0]) printf("%s\n", s->message);

    if (s->step == SETUP_PICK_SUBJECT) printf("과목 선택 : ");
    else if (s->step == SETUP_NEW_SUBJECT) printf("새 과목명(1~20자) : ");
    else printf("목표 시간(분) : ");
    fflush(stdout);
}

int setup_run(Session* plan)
{
    SetupState s;
    char line[256];

    setup_init(&s);
    do {
        setup_print(&s);
        if (!fgets(line, sizeof(line), stdin)) return 0;
    } while (!setup_submit(&s, line, plan));

    printf("\nEnter 키를 누르면 학습을 시작합니다.");
    // 여기서 입력이 끊겨도 설정은 끝났으니 그대로 시작함
    fgets(line, sizeof(line), stdin);
    return 1;
}