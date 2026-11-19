#ifndef DETECT_H
#define DETECT_H

#include "common.h"

int get_keyword_sec(int index);

int detect_current_window(void);

// 이탈 상태 초기화
void init_detect(void);

// 이탈 시간 및 횟수 계산
void update_detect(Session* rec, int elapsed);

#endif