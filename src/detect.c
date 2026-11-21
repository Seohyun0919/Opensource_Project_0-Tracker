#include <string.h>
#include <windows.h>

#include "detect.h"
#include "keyword.h"
#include "warning.h"

#define TITLE_SIZE (MAX_TITLE_LEN * 3 + 1)

static int was_distract = 0;
static int candidate_start = -1;
static int last_sec = -1;
static int distract_total = 0;

static int keyword_sec[MAX_KEYWORDS] = {0};
static int candidate_sec[MAX_KEYWORDS] = {0};
static int last_index = -1;

// 영문 대문자만 소문자로 바꿈
// 예를 들어 창 제목에 YOUTUBE가 있어도 YouTube 키워드에 걸리게 함
static void make_lower(char text[])
{
    for (int i = 0; text[i] != '\0'; i++)
    {
        if (text[i] >= 'A' && text[i] <= 'Z')
        {
            text[i] += 'a' - 'A';
        }
    }
}

// 현재 활성 창을 한 번 검사함
// 키워드에 걸리면 목록에서 몇 번째 칸인지 돌려줌 (첫 번째 칸은 0)
// 걸리는 키워드가 없으면 -1을 돌려줌
// 창 제목이 비어 있거나 읽지 못하면 -2를 돌려줌 
int detect_current_window(void)
{
    wchar_t wide_title[MAX_TITLE_LEN + 1];
    char title[TITLE_SIZE];

    // GetForegroundWindow()로 사용자가 현재 보고 있는 활성 창을 가져옴
    HWND window = GetForegroundWindow();

    // 창이 없거나 제목을 읽지 못하면 비교할 수 없으므로 여기서 끝냄
    // -1(학습) 대신 -2를 돌려줌 
    // 바탕화면, 작업 표시줄처럼 제목이 비어있는 창을 학습으로 인식하지 않고 바로 이전 상태로 이어가기 위함  
    if (window == NULL ||
        GetWindowTextW(window, wide_title, MAX_TITLE_LEN + 1) == 0)
    {
        return -2;
    }

    // Windows에서 읽어 온 제목을 키워드와 같은 UTF-8 문자열로 바꿈
    if (WideCharToMultiByte(CP_UTF8, 0, wide_title, -1,
                            title, sizeof(title), NULL, NULL) == 0)
    {
        return -1;
    }
    if (strcmp(title, "0:Tracker WARNING") == 0)
    {
        return -2;
    }

    make_lower(title);

    // 목록 앞에서부터 창 제목에 들어 있는 키워드를 찾음
    for (int i = 0; i < keyword_count(); i++)
    {
        char keyword[KEYWORD_SIZE];

        // 저장된 키워드 표기는 그대로 두고 비교용 복사본만 소문자로 바꿈
        strcpy(keyword, keyword_at(i));
        make_lower(keyword);

        // strstr()은 제목에 키워드가 들어 있으면 그 위치를 돌려줌
        // 여러 키워드가 들어 있으면 목록에서 먼저 찾은 칸을 돌려줌
        if (strstr(title, keyword) != NULL)
        {
            return i;
        }
    }

    return -1;
}

void init_detect(void)
{
    was_distract = 0;
    candidate_start = -1;
    last_sec = -1;
    distract_total = 0;

    last_index = -1;

    for (int i = 0; i < MAX_KEYWORDS; i++)
    {
        keyword_sec[i] = 0;
        candidate_sec[i] = 0;
    }
}

void update_detect(Session* rec, int elapsed)
{
    int index;

    // 1초에 한 번만 검사
    if (last_sec == elapsed)
        return;

    // 직전 키워드의 체류 시간 누적
    if (last_sec >= 0 && last_index >= 0)
    {
        int diff = elapsed - last_sec;

        if (was_distract)
            keyword_sec[last_index] += diff;
        else
            candidate_sec[last_index] += diff;
    }

    last_sec = elapsed;

    index = detect_current_window();

    // 이탈 키워드 감지
    if (index >= 0)
    {
        if (candidate_start == -1)
            candidate_start = elapsed;

        // 5초 이상 지속되면 이탈로 판단
        if (was_distract == 0 &&
            elapsed - candidate_start >= DISTRACT_GRACE_SEC)
        {
            rec->distract_count++;
            was_distract = 1;

            // 유예 기간의 키워드별 시간도 포함
            for (int i = 0; i < MAX_KEYWORDS; i++)
            {
                keyword_sec[i] += candidate_sec[i];
                candidate_sec[i] = 0;
            }

            warning();
        }

        last_index = index;
    }

    // 정상 학습 복귀
    else if (index == -1)
    {
        if (was_distract == 1)
        {
            distract_total += elapsed - candidate_start;
        }

        was_distract = 0;
        candidate_start = -1;
        last_index = -1;

        // 확정되지 않은 후보 시간 제거
        for (int i = 0; i < MAX_KEYWORDS; i++)
            candidate_sec[i] = 0;
    }

    // 누적 이탈 시간 계산
    rec->distract_sec = distract_total;

    if (was_distract == 1)
    {
        rec->distract_sec += elapsed - candidate_start;
    }
}

// 해당 키워드의 누적 이탈 시간 반환
int get_keyword_sec(int index)
{
    if (index < 0 || index >= keyword_count())
        return 0;

    return keyword_sec[index];
}