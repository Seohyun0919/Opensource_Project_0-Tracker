#ifndef STUDY_UI_H
#define STUDY_UI_H

#include "common.h"

// Enter로 시작하고 Q로 종료할 때까지 학습 화면을 갱신한다.
void study_ui(Session* rec, int resume); // resume이 1이면 rec에 담긴 기록을 이어서 센다. 0이면 새로 시작한다.

#endif
