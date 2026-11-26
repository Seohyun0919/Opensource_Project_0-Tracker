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