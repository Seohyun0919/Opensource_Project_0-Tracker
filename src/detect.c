#include <stdio.h>
#include <string.h>
#include <ctype.h> // tolower(대문자 > 소문자)를 사용하기 위해서
#include <windows.h> // 창 제목을 읽고, Sleep을 사용하기 위해서
#include "detect.h" // 직접 만든 헤더파일 
#include <conio.h> // 키보드 입력을 확인해주는 _kbhit, _getch를 사용하기 위해서


// 이탈 키워드 목록과 개수
char distract_keywords[MAX_KEYWORDS][MAX_KEYWORD_LEN] = {"YouTube", "Instagram","Facebook"}; 
int distract_keyword_count = 3;


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
    if(GetWindowTextW(GetForegroundWindow(), wide_title, 300) == 0)
    {
        return;
    }

    // 한글이 깨지지 않게 하기 위해서 UTF-8문자열로 바꿈
    WideCharToMultiByte(CP_UTF8, 0, wide_title, -1, window_title, MAX_TITLE_LEN, NULL,NULL);
}

// 문자열의 영문 대문자를 소문자로 변환
void make_lower(char text[])
{
    int i;

    for(i = 0; text[i] != '\0'; i++)
    {
        // 글자가 음수로 처리되지 않도록 함 
        text[i] = (char)tolower((unsigned char)text[i]);
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

    for(i = 0; i < distract_keyword_count; i++)
    {
        strcpy(keyword, distract_keywords[i]);
        make_lower(keyword);

        // strstr() title 안에 keyword가 있으면 title에 대한 위치를 돌려줌
        // 없으면 NULL을 돌려줌.
        if(strstr(title, keyword) != NULL)
        {
            return 1;
        }
    }
    return 0;
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
    int study_sec = 0; // 공부한 시간 (초)
    int distract_sec = 0; // 이탈한 시간 (초)
    int distract_count = 0; // 이탈한 횟수 
    int was_distract = 0; // 바로 전에 이탈 중이었으면 1
    char key;

    SetConsoleOutputCP(CP_UTF8);
    printf("!!기록 시작!! (끝내려면 창에서 Q를 누르세요) \n\n");

    while(1)
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
