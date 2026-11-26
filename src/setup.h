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
    int scroll;                      // 목록 맨 위에 보이는 과목의 배열 번호
    char input[SETUP_SUBJECT_SIZE];
    char message[80];                // 잘못 입력했을 때 안내
} SetupState;

void setup_init(SetupState* s);
// 목표 시간까지 입력하고 Enter를 누르면 1, 그 전에는 0. plan에 과목·날짜·목표 시간을 채움
int setup_update(SetupState* s, Session* plan);
void setup_draw(const SetupState* s);

#endif