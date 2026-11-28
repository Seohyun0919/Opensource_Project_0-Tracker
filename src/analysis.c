// M4 결과 분석. M3가 읽어 준 기록으로 계산하고 결과는 M7 결과 화면이 가져감
#include <stdlib.h>
#include <string.h>
#include "common.h"

// 세션 번호로 바로 찾는 M3 함수가 없어서 전부 읽어 찾음. 없으면 0
static int findSessionByNum(int session_num, Session* out)
{
    Session* list;
    int count;
    int found;
    int i;
    int ok = 0;

    count = loadSessions(NULL, 0);
    if (count == 0) {
        return 0;
    }
    list = malloc(count * sizeof(Session));
    if (list == NULL) {
        return 0;
    }
    found = loadSessions(list, count);
    if (found < count) {
        count = found;
    }
    for (i = 0; i < count; i++) {
        if (list[i].session_num == session_num) {
            *out = list[i];
            ok = 1;
            break;
        }
    }
    free(list);
    return ok;
}

void analyzeSession(int session_num, AnalysisResult* result)
{
    Session s;
    int total_sec;

    memset(result, 0, sizeof(*result));
    // 창 전환 기록이 아니라 세션 값을 씀. 5초 유예가 적용된 값이라 학습 중 화면 숫자와 같음
    if (!findSessionByNum(session_num, &s)) {
        return;
    }
    result->total_study_sec = s.study_sec;
    result->total_distract_sec = s.distract_sec;
    result->total_distract_count = s.distract_count;

    total_sec = s.study_sec + s.distract_sec;
    if (total_sec > 0) {
        result->focus_rate = s.study_sec * 100.0 / total_sec;
    }
}
