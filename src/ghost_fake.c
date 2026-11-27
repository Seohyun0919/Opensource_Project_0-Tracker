// 가짜 기록 파일임 (혼자 테스트할 때 필요하여 만듦)
// 기록 담당(M3)의 진짜 findBestSession, loadWindowLogs가 deveop에 합쳐지면 이 파일 지움 
#include <string.h>
#include "common.h"

int findBestSession(const char* subject, Session* best)
{
    if (strcmp(subject, "오픈소스") == 0)
    {
        best -> session_num = 1;
        strcpy(best-> subject, "오픈소스");
        best -> study_sec = 240;
        return 1;
    }
    return 0;
}

int loadWindowLogs(int session_num, WindowLog* list, int max)
{
    if (session_num != 1 || max < 3)
    {
        return 0;
    }

    strcpy(list[0].switch_time, "10:00:00");
    strcpy(list[0].window_title, "detect.c - Visual Studio Code");
    list[0].stay_sec = 120;
    list[0].is_distract = 0;

    strcpy(list[1].switch_time, "10:02:00");
    strcpy(list[1].window_title, "아따맘마 - YouTube");
    list[1].stay_sec = 60;
    list[1].is_distract = 1;

    strcpy(list[2].switch_time, "10:03:00");
    strcpy(list[2].window_title, "detect.c - Visual Studio Code");
    list[2].stay_sec = 120;
    list[2].is_distract = 0;

    return 3;
}