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

// 이탈 없이 이어진 창들을 한 구간으로 묶음. 구간이 없으면 0
static int calcAvgFocusSec(const WindowLog* logs, int count)
{
    int i;
    int focus_sum = 0;
    int segments = 0;

    for (i = 0; i < count; i++) {
        if (logs[i].is_distract) {
            continue;
        }
        focus_sum += logs[i].stay_sec;
        if (i == 0 || logs[i - 1].is_distract) {
            segments++;
        }
    }
    return segments > 0 ? focus_sum / segments : 0;
}

// 제목이 같은 이탈 창끼리 머문 시간을 더해 가장 긴 창을 고름. 시간이 같으면 먼저 나온 창
// 주의: 제목이 완전히 같아야 같은 창. 영상마다 제목이 바뀌는 YouTube 탭은 따로 셈
static void findTopDistractWindow(const WindowLog* logs, int count, char* out, size_t out_size)
{
    int i;
    int j;
    int sum;
    int best_sum = 0;
    int seen;

    for (i = 0; i < count; i++) {
        if (!logs[i].is_distract) {
            continue;
        }
        seen = 0;
        for (j = 0; j < i && !seen; j++) {
            seen = logs[j].is_distract && strcmp(logs[j].window_title, logs[i].window_title) == 0;
        }
        if (seen) {
            continue;
        }
        sum = 0;
        for (j = i; j < count; j++) {
            if (logs[j].is_distract && strcmp(logs[j].window_title, logs[i].window_title) == 0) {
                sum += logs[j].stay_sec;
            }
        }
        if (sum > best_sum) {
            best_sum = sum;
            strcpy_s(out, out_size, logs[i].window_title);
        }
    }
}

static void analyzeWindowLogs(int session_num, AnalysisResult* result)
{
    WindowLog* logs;
    int count;
    int found;

    count = loadWindowLogs(session_num, NULL, 0);
    if (count == 0) {
        return;
    }
    logs = malloc(count * sizeof(WindowLog));
    if (logs == NULL) {
        return;
    }
    found = loadWindowLogs(session_num, logs, count);
    if (found < count) {
        count = found;
    }
    result->avg_focus_sec = calcAvgFocusSec(logs, count);
    findTopDistractWindow(logs, count, result->top_distract_window,
                          sizeof(result->top_distract_window));
    free(logs);
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
    analyzeWindowLogs(session_num, result);
}
