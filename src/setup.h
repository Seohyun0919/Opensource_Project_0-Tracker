// setup.h
#ifndef SETUP_H
#define SETUP_H

#include "common.h"

#define SETUP_MAX_SUBJECTS  30
#define SETUP_SUBJECT_SIZE  (MAX_SUBJECT_LEN * 3 + 1)

typedef enum { SETUP_PICK_SUBJECT, SETUP_NEW_SUBJECT, SETUP_GOAL } SetupStep;

typedef struct {
    SetupStep step;
    char date[11];
    char subjects[SETUP_MAX_SUBJECTS][SETUP_SUBJECT_SIZE];
    int subject_count;
    int chosen;                      // 고른 과목의 배열 번호, 아직 없으면 -1
    char message[80];                // 잘못 입력했을 때 안내
} SetupState;

void setup_init(SetupState* s);
// 한 줄을 입력받아 현재 단계에 넘김. 목표 시간까지 받으면 1, 그 전에는 0.
// plan에 과목·날짜·목표 시간을 채움. 키 입력 방식(콘솔·raylib)과 상관없이 쓰려고 분리함
int setup_submit(SetupState* s, const char* line, Session* plan);
// 콘솔 화면. raylib 화면이 합쳐지면 setup_submit만 남기고 지움
void setup_print(const SetupState* s);
// 입력이 끊기면(EOF) 0, 시작까지 끝나면 1
int setup_run(Session* plan);

#endif