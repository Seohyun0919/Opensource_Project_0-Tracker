#ifndef GHOST_H
#define GHOST_H

#include "common.h"

// 1분 중에 이 시간 이상 학습 이탈을 하면 그 시간 1분을 학습 이탈로 인식 
#define GHOST_DISTRACT_SEC 30

// 타임라인에 담을 수 있는 최대 분 (10시간)
#define GHOST_MAX_MIN 600

// 끝났을 때 결과 화면에 넘길 값
typedef struct 
{
    int has_best; // 최고 기록이 있었다면 1 & 최고 기록이 없었다면 0
    int today_sec; // 오늘의 순수 학습 시간
    int best_sec; // 기존의 최고 기록 시간
    int is_new; // 최고 기록을 갱신했다면 1 
} GhostResult;

int build_timeline(const WindowLog list[], int count, int timeline[], int max_min);
int load_ghost(const char* subject, Session* best, int best_timeline[], int* best_min);
// 학습을 시장할 때 부름 
int init_ghost(const char* subject);
// 1초마다 부름 -> 메세지를 띄울 게 없다면 NULL
const char* update_ghost(const Session* rec, int elapsed);
// 공부가 끝나면 부름 
GhostResult finish_ghost(const Session* today);
#endif

