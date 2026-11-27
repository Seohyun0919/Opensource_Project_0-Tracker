#include <stdio.h>
#include <string.h>
#include "ghost.h"

#define GHOST_CHEER_TEXT "최고 기록 때는 학습 이탈 중이었는데, 지금은 집중하고 있어요! 화이팅!"
#define GHOST_PUSH_TEXT "최고 기록 때는 이 시간에 학습하고 있었어요. 다시 학습하러 가세요!"

// 고스트 상태 
static Session ghost_best; // 최고 기록 
static int ghost_timeline[GHOST_MAX_MIN]; // 최고 기록의 1분 타임라인
static int ghost_min = 0; // 최고 기록이 몇 분인지 
static int has_ghost = 0; // 최고 기록이 있으면 1
static int last_text = 0; // 마지막으로 띄운 메시지 (0= 없음, 1= 격려, 2= 자극)
static int last_sec = -1; // 마지막으로 검사한 시각 
static int last_distract_sec = 0; // 1초 전의 누적 이탈 시간 

int build_timeline(const WindowLog list[], int count, int timeline[], int max_min)
{
    int sec = 0;
    int minute;
    int used_min = 0;
    int i;
    int s;

    for (i = 0; i < max_min; i++)
    {
        timeline[i] = 0;
    }

    for (i = 0; i < count; i++)
    {
        for (s = 0; s < list[i].stay_sec; s++)
        {
            minute = sec / 60;

            if (minute >= max_min)
            {
                break;
            }

            if (list[i].is_distract == 1)
            {
                timeline[minute]++;
            }

            sec++;
        }
    }

    used_min = (sec + 59) / 60;
    if (used_min > max_min)
    {
        used_min = max_min;
    }

    for (i = 0; i < used_min; i++)
    {
        if (timeline[i] >= GHOST_DISTRACT_SEC)
        {
            timeline[i] = 0;
        }

        else 
        {
            timeline[i] = 1;
        }
    }
    
    return used_min;
}

int makeTimeline(int session_num, int* timeline, int max_min)
{
    static WindowLog list[2000];
    int count;

    count = loadWindowLogs(session_num, list, 2000);
    
    return build_timeline(list, count, timeline, max_min);
}

int load_ghost(const char* subject, Session* best, int best_timeline[], int* best_min)
{
    *best_min = 0;

    if (findBestSession(subject, best) == 0)
    {
        printf("[고스트] '%s' 과목은 최고 기록 없습니다. 비교 없이 진행합니다.\n", subject);
        return 0;
    }

    *best_min = makeTimeline(best->session_num, best_timeline, GHOST_MAX_MIN);

    printf("[고스트] '%s' 최고 기록을 불러왔습니다. (%d분)\n", subject, *best_min);
    return 1;
}

// 같은 과목의 최고 기록을 가져오고 상태를 처음으로 되돌림
int init_ghost(const char* subject)
{
    has_ghost = load_ghost(subject, &ghost_best, ghost_timeline, &ghost_min);
    last_text = 0;
    last_sec = -1;
    last_distract_sec = 0;

    return has_ghost;
}

// 1초마다 부름
// 같은 시간대(몇 분)의 최고 기록과 현재를 비교하여 상태가 바뀔때만 메시지를 줌 (메시지 띄울 게 없다면 NULL)
const char* update_ghost(const Session* rec, int elapsed)
{
    int minute;
    int now_distract;
    int text = 0;

    // 최고 기록이 없다면 비교 X
    if (has_ghost == 0)
    {
        return NULL;
    }

    // 1초에 한 번씩만 검사 
    if (last_sec == elapsed)
    {
        return NULL;
    }
    last_sec = elapsed;

    // 누적 이탈 시간이 1초 전보다 늘었다면 현재 학습 이탈 중 
    now_distract = (rec -> distract_sec > last_distract_sec);
    last_distract_sec = rec -> distract_sec;

    // 현재 몇 분째인지 (0부터)
    minute = elapsed / 60;

    // 최고 기록보다 오래 학습하는 중이라면 비교할 칸이 없음 
    if (minute >= ghost_min)
    {
        return NULL;
    }

    if (ghost_timeline[minute] == 0 && now_distract == 0)
    {
        text = 1; // 최고 기록 때는 학습 이탈 중이었으나 현재는 학습중 -> 격려 메세지 출력 
    }

    else if (ghost_timeline[minute] == 1 && now_distract == 1)
    {
        text = 2; // 최고 기록 때는 학습 중이었으나 현재는 학습 이탈중 -> 자극 메세지 출력 
    }

    // 상태가 바뀌지 않았다면 메세지를 출력하지 않음
    if (text == last_text)
    {
        return NULL;
    }
    last_text = text;

    if (text == 1)
    {
        return GHOST_CHEER_TEXT;
    }

    else if (text == 2)
    {
        return GHOST_PUSH_TEXT;
    }

    return NULL;   
}

GhostResult finish_ghost(const Session* today)
{
    GhostResult result;

    result.has_best = has_ghost;
    result.today_sec = today->study_sec;
    result.best_sec = 0;
    result.is_new = 0;

    if (has_ghost == 1)
    {
        result.best_sec = ghost_best.study_sec;
    }

    // 오늘 학습 기록이 더 길다면 갱신 (짧거나 같다면 그대로 두기)
    // 최고 기록은 세션 기록 중 가장 긴 것을 찾아서 사용함
    // 오늘 세션이 저장되면 다음부터 자동으로 새 최고 기록으로 바뀜 
    if (today->study_sec > result.best_sec)
    {
        result.is_new = 1;
    }

    return result;
}

#ifdef GHOST_TEST
int main(void)
{
    Session best;
    int timeline[GHOST_MAX_MIN];
    int best_min;
    Session today = {0};
    GhostResult result;
    const char* text;
    int t;
    int i;
    
    if (load_ghost("오픈소스", &best, timeline, &best_min) == 1)
    {
        printf("분:   ");
        for (i = 0; i < best_min; i++)
        {
            printf("%d ", i + 1);
        }

        printf("\n상태:  ");
        for(i = 0; i < best_min; i++)
        {
            printf("%d ", timeline[i]);
        }
        printf(" (1= 집중, 0= 이탈)\n\n");
    }

    // 가짜로 5분 공부해봄
    // 1분 30초 ~ 2분 30초 딴짓, 나머지는 집중
    init_ghost("오픈소스");
    strcpy(today.subject, "오픈소스");
    for(t=0; t < 300; t++)
    {
        if (t >= 90 && t < 150)
        {
            today.distract_sec++;
        }

        text = update_ghost(&today, t);
        if (text != NULL)
        {
            printf("[%d분 %02d초] %s\n", t / 60, t % 60, text);
        }
    }

    // 끝났을 때 결과
    today.study_sec = 300 - today.distract_sec;
    result = finish_ghost(&today);
    printf("\n오늘 %d초 / 최고 기록 %d초\n", result.today_sec, result.best_sec);
    if (result.is_new == 1)
    {
        printf("최고 기록 갱신\n");
    }

    else 
    {
        printf("최고 기록 유지\n");
    }

    // 기록 없는 과목도 init_ghost로 확인함 
    printf("\n");
    init_ghost("데이터구조");

    return 0;
}
#endif
