#include <stdio.h>
#include <string.h>
#include <windows.h> // 창 제목을 읽고, Sleep을 사용하기 위해서
#include "detect.h"  // 직접 만든 헤더파일
#include <conio.h>   // 키보드 입력을 확인해주는 _kbhit, _getch를 사용하기 위해서

// 이탈 키워드 목록과 개수
char distract_keywords[MAX_KEYWORDS][MAX_KEYWORD_LEN];
int distract_keyword_count = 0;
char default_keywords[DEFAULT_KEYWORD_COUNT][MAX_KEYWORD_LEN] = {"YouTube", "카카오톡", "Instagram", "Facebook"};

// 현재 맨 앞에 있는 창(=활성 창)의 제목을 window_title에 저장
// if.제목을 제대로 읽지 못함 -> 빈 문자열
void get_window_title(char window_title[])
{
    // 한글도 담을 수 있는 유니코드 문자 배열
    wchar_t wide_title[300];

    // 제목을 못 읽을 때를 위한 빈 문자열
    window_title[0] = '\0';

    // GetForegroundWindow() 맨 앞에 있는 창을 찾음
    // GetWindowTextW() 창의 제목을 읽고, 읽은 글자 수를 다시 돌려줌 (0이면 실패임)
    if (GetWindowTextW(GetForegroundWindow(), wide_title, 300) == 0)
    {
        return;
    }

    // 한글이 깨지지 않게 하기 위해서 UTF-8문자열로 바꿈
    WideCharToMultiByte(CP_UTF8, 0, wide_title, -1, window_title, MAX_TITLE_LEN, NULL, NULL);
}

// 문자열의 영문 대문자를 소문자로 변환
void make_lower(char text[])
{
    int i;

    for (i = 0; text[i] != '\0'; i++)
    {
        if (text[i] >= 'A' && text[i] <= 'Z')
        {
            text[i] = text[i] + ('a' - 'A');
        }
    }
}

// 창 제목에 이탈 키워드가 하나라도 있다면 1을 줌 or 없으면 0을 돌려줌
// 제목과 키워드를 모두 소문자로 바꿔서 비교함
int is_distract_title(char window_title[])
{
    char title[MAX_TITLE_LEN];
    char keyword[MAX_KEYWORD_LEN];
    int i;

    // 원본 제목은 출력할 때 필요. 복사본을 소문자로 변환
    strcpy(title, window_title);
    make_lower(title);

    for (i = 0; i < distract_keyword_count; i++)
    {
        strcpy(keyword, distract_keywords[i]);
        make_lower(keyword);

        // strstr() title 안에 keyword가 있으면 title에 대한 위치를 돌려줌
        // 없으면 NULL을 돌려줌.
        if (strstr(title, keyword) != NULL)
        {
            return 1;
        }
    }
    return 0;
}

// fgets로 한 줄을 읽으면 맨 마지막에 줄바꿈 문자가 같이 들어옴
// -> 문자열 끝의 줄바꿈 문자를 지움
void remove_newline(char text[])
{
    int i;

    for (i = 0; text[i] != '\0'; i++)
    {
        if (text[i] == '\n' || text[i] == '\r')
        {
            text[i] = '\0';
            return;
        }
    }
}
// 목록에 키워드가 있는지 없는지 확인
// 목록에 있는 키워드면 1, 목록에 없는 키워드면 0
int has_keyword(char word[])
{
    int i;

    for (i = 0; i < distract_keyword_count; i++)
    {
        if (strcmp(distract_keywords[i], word) == 0)
        {
            return 1;
        }
    }
    return 0;
}

// 기본 키워드(유튜브, 카톡, 인스타, 페이스북)을 항상 먼저 넣음
// 그리고 나서 사용자가 추가한 이탈 키워드를 파일에서 불러옴
void load_keywords(void)
{
    FILE *fp;
    char line[MAX_KEYWORD_LEN];
    int i;

    // 기본 키워드 4개를 항상 먼저 넣음
    for (i = 0; i < DEFAULT_KEYWORD_COUNT; i++)
    {
        strcpy(distract_keywords[i], default_keywords[i]);
    }
    distract_keyword_count = DEFAULT_KEYWORD_COUNT;

    // 사용자가 추가한 이탈 키워드를 파일에서 읽음
    // 파일에 없다면 기본 4개만 이탈 키워드가 됨
    fp = fopen(FILE_DISTRACT_KEYWORDS, "r");
    if (fp == NULL)
    {
        return;
    }

    // 파일을 한 줄 씩 읽으면서 목록에 추가
    while (fgets(line, sizeof(line), fp) != NULL)
    {
        remove_newline(line);

        // 목록이 다 차면 읽는 것을 멈춤
        if (distract_keyword_count >= MAX_KEYWORDS)
        {
            break;
        }

        // 줄이 비어있지 않고, 아직 키워드가 없다면
        if (line[0] != '\0' && has_keyword(line) == 0)
        {
            strcpy(distract_keywords[distract_keyword_count], line);
            distract_keyword_count++;
        }
    }
    fclose(fp);
}

// 사용자가 추가한 키워드만 파일에 저장함
void save_keywords(void)
{
    FILE *fp;
    int i;

    // w을 사용하여 기존에 있던 내용를 지우고 새로 씀
    fp = fopen(FILE_DISTRACT_KEYWORDS, "w");

    if (fp == NULL)
    {
        printf("파일 저장 불가\n");
        return;
    }

    for (i = DEFAULT_KEYWORD_COUNT; i < distract_keyword_count; i++)
    {
        // 한 줄에 키워드를 하나씩 사용함
        fprintf(fp, "%s\n", distract_keywords[i]);
    }

    fclose(fp);
}

// 키워드 목록 출력
void print_keywords(void)
{
    int i;

    printf("\n[이탈 키워드 목록]\n");

    for (i = 0; i < distract_keyword_count; i++)
    {
        if (i < DEFAULT_KEYWORD_COUNT)
        {
            printf(" %d. %s (기본 이탈 키워드)\n", i+1, distract_keywords[i]);
        }

        else 
        {
            printf(" %d. %s\n", i+1, distract_keywords[i]);
        }
    }
}

// 단어가 목록의 몇 번째 칸에 있는지 찾음
// 찾으면 칸 번호, 없다면 -1
int find_keyword(char word[])
{
    int i;

    for (i = 0; i < distract_keyword_count; i++)
    {
        if (strcmp(distract_keywords[i], word) == 0)
        {
            return i;
        }
    }
    return -1;
}


// 사용자가 키워드를 입력 > 목록에 추가 
void add_keyword(void)
{
    char word[MAX_KEYWORD_LEN];

    printf("키워드 추가>> ");
    fgets(word, MAX_KEYWORD_LEN, stdin);
    remove_newline(word);

    // 아무것도 입력받지 않았으면 추가하지 않음 
    if (word[0] == '\0')
    {
        printf("키워드가 입력되지 않았습니다.\n");
        return;
    }

    // 이미 있는 키워드라면 추가하지 않음 
    if (has_keyword(word) == 1)
    {
        printf("이미 있는 키워드 입니다.\n");
        return;
    }

    // 목록이 가득 찼다면 추가하지 않음 
    if (distract_keyword_count >= MAX_KEYWORDS)
    {
        printf("목록이 가득 찼습니다.\n");
        return;
    }

    // 마지막 키워드 다음 칸에 새로운 키워드를 넣고 키워드 개수를 하나 늘림
    strcpy(distract_keywords[distract_keyword_count], word);
    distract_keyword_count++;
    save_keywords();

    printf(" '%s'을 추가하였습니다.\n", word);
}


// 시간을 초로 받은 후에 "n시간 n분 n초" 형식으로 출력
void print_time(int total_sec)
{
    int hour = total_sec / 3600;
    int min = (total_sec % 3600) / 60;
    int sec = total_sec % 60;

    if (hour > 0)
    {
        printf("%d시간 %d분 %d초", hour, min, sec);
    }
    else if (min > 0)
    {
        printf("%d분 %d초", min, sec);
    }
    else
    {
        printf("%d초", sec);
    }
}

int main(void)
{
    char window_title[MAX_TITLE_LEN];
    int study_sec = 0;      // 공부한 시간 (초)
    int distract_sec = 0;   // 이탈한 시간 (초)
    int distract_count = 0; // 이탈한 횟수
    int was_distract = 0;   // 바로 전에 이탈 중이었으면 1
    char key;

    SetConsoleOutputCP(CP_UTF8);
    load_keywords();
    printf("!!기록 시작!! (끝내려면 창에서 Q를 누르세요) \n\n");

    while (1)
    {
        get_window_title(window_title);

        if (is_distract_title(window_title))
        {
            distract_sec++;

            // 학습 > 이탈로 바뀌었을때 횟수를 셈 + 출력
            if (was_distract == 0)
            {
                distract_count++;
                printf("[이탈 시작] %s\n", window_title);
            }

            was_distract = 1;
        }
        else
        {
            study_sec++;

            // 이탈 > 학습으로 바뀌었을 때 출력
            if (was_distract == 1)
            {
                printf("[학습 복귀] %s\n", window_title);
            }
            was_distract = 0;
        }

        // Q를 누르면 기록을 끝냄
        if (_kbhit())
        {
            key = _getch();
            if (key == 'q' || key == 'Q')
            {
                break;
            }
        }

        // 1초 기다리기 (반복 1번은 1초가 되도록 함)
        Sleep(1000);
    }

    printf("\n========= 결과 =========\n");

    printf("전체 시간 : ");
    print_time(study_sec + distract_sec);
    printf("\n");

    printf("공부한 시간 : ");
    print_time(study_sec);
    printf("\n");

    printf("이탈한 시간 : ");
    print_time(distract_sec);
    printf("\n");

    printf("이탈 횟수 : %d회\n", distract_count);
    printf("======================\n");

    return 0;
}
