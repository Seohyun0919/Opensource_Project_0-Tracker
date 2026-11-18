#include <stdio.h>
#include <string.h>
#include <windows.h> // 창 제목을 읽고, Sleep을 사용하기 위해서
#include "detect.h"  // 직접 만든 헤더파일
#include <conio.h>   // 키보드 입력을 확인해주는 _kbhit, _getch를 사용하기 위해서

#define KEYWORD_SIZE (MAX_KEYWORD_LEN * 3 + 1)
#define TITLE_SIZE (MAX_TITLE_LEN * 3 + 1)

// 이탈 키워드 목록과 개수
char distract_keywords[MAX_KEYWORDS][KEYWORD_SIZE];
int distract_keyword_count = 0;
char default_keywords[DEFAULT_KEYWORD_COUNT][KEYWORD_SIZE] = {"YouTube", "카카오톡", "Instagram", "Facebook"};

// 현재 맨 앞에 있는 창(=활성 창)의 제목을 window_title에 저장
// if.제목을 제대로 읽지 못함 -> 빈 문자열
void get_window_title(char window_title[])
{
    // 한글도 담을 수 있는 유니코드 문자 배열
    wchar_t wide_title[MAX_TITLE_LEN+1];

    // 제목을 못 읽을 때를 위한 빈 문자열
    window_title[0] = '\0';

    // GetForegroundWindow() 맨 앞에 있는 창을 찾음
    // GetWindowTextW() 창의 제목을 읽고, 읽은 글자 수를 다시 돌려줌 (0이면 실패임)
    if (GetWindowTextW(GetForegroundWindow(), wide_title, MAX_TITLE_LEN + 1) == 0)
    {
        return;
    }

    // 한글이 깨지지 않게 하기 위해서 UTF-8문자열로 바꿈
    WideCharToMultiByte(CP_UTF8, 0, wide_title, -1, window_title, TITLE_SIZE, NULL, NULL);
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
    char title[TITLE_SIZE];
    char keyword[KEYWORD_SIZE];
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


// 창 제목에 들어 있는 이탈 키워드가 몇 번째 키워드인지 찾음
// 찾으면 키워드 번호, 찾지 못하면 -1
int find_distract_index(char window_title[])
{
    char title[TITLE_SIZE];
    char keyword[KEYWORD_SIZE];
    int i;

    strcpy(title, window_title);
    make_lower(title);

    for (i = 0; i < distract_keyword_count; i++)
    {
        strcpy(keyword, distract_keywords[i]);
        make_lower(keyword);

        if(strstr(title, keyword) != NULL)
        {
            return i;
        }
    }
    return -1;
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
    char line[KEYWORD_SIZE];
    int i;

    // default_keywords에 있는 기본 키워드 4개를
    // distract_keywords로 하나씩 옮김
    for (i = 0; i < DEFAULT_KEYWORD_COUNT; i++)
    {
        // i번째 키워드도 똑같이 옮겨서 씀 
        strcpy(distract_keywords[i], default_keywords[i]);
    }
    // 지금 상자에 키워드 가 4개 들어있다고 기록 
    // 지금 빈칸 수를 세야 나중에 사용자가 키워드 직접 추가할때 다음 빈칸인 4번 칸에 들어감 
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
        // 꺼낸 줄 끝에 줄바꿈 엔터가 붙는데  지움 
        remove_newline(line);

        // 목록(20개)이 다 차면 읽는 것을 멈춤
        if (distract_keyword_count >= MAX_KEYWORDS)
        {
            break;
        }

        // 줄이 비어있지 않고, 아직 키워드가 없다면
        if (line[0] != '\0' && has_keyword(line) == 0)
        {
            // 다음 빈칸에 키워드를 넣음 
            strcpy(distract_keywords[distract_keyword_count], line);
            // 상자 안에 키워드를 하나 늘림 
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
    char word[KEYWORD_SIZE];

    printf("키워드 추가>> ");
    fgets(word, KEYWORD_SIZE, stdin);
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


// 키워드를 입력 받은 후에 목록에서 삭제
void remove_keyword(void)
{
    char word[KEYWORD_SIZE];
    int index; // 지우려는 키워드가 있는 칸 인덱스 
    int i;

    printf("키워드 삭제>> ");
    fgets(word, KEYWORD_SIZE, stdin);
    remove_newline(word);

    index = find_keyword(word);

    // 목록에 키워드가 없으면 삭제하지 않음
    if (index == -1)
    {
        printf("키워드가 목록에 없습니다.\n");
        return;
    }

    // 0~3번째 인덱스는 기본 키워드이므로 삭제할 수 없음 
    if (index < DEFAULT_KEYWORD_COUNT)
    {
        printf("기본 키워드는 삭제할 수 없습니다.\n");
        return;
    }

    // 지우려는 킨부터 뒤의 키워드를 하나씩 앞으로 당겨옴
    // ex. [a][b][c][d][e] > c 삭제 > [a][b][d][e]
    for (i = index; i < distract_keyword_count - 1; i++)
    {
        strcpy(distract_keywords[i], distract_keywords[i+1]);
    }

    // 개수를 하나 줄임 -> 파일에 저장 
    distract_keyword_count --;
    save_keywords();

    printf("'%s' 키워드를 삭제했습니다.\n", word);

}


// 키워드 관리하는 메뉴
// 1을 누름: 추가, 2를 누름: 삭제, 3을 누름: 메뉴 종료 및 기록 시작
void keyword_menu(void)
{
    char menu[10];

    while(1)
    {
        print_keywords();
        printf("1= 추가 | 2= 삭제 | 3= 메뉴 종료 및 기록 시작\n");
        printf("선택하세요>> ");

        fgets(menu, 10, stdin);

        if (menu[0] == '1')
        {
            add_keyword();
        }
        
        else if (menu[0] == '2')
        {
            remove_keyword();
        }

        else if (menu[0] == '3')
        {
            return;
        }

        else
        {
            printf("잘못된 선택입니다. 1,2,3 중에서 선택 해주세요.\n");
        }
    }
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

// 단독 테스트용 main. 통합 빌드(main.c)에서는 빠지도록 매크로로 감쌈
// 단독으로 돌려 보려면 컴파일할 때 /DDETECT_STANDALONE 을 붙임
#ifdef DETECT_STANDALONE
int main(void)
{
    char window_title[TITLE_SIZE];
    int study_sec = 0;      // 공부한 시간 (초)
    int distract_sec = 0;   // 이탈한 시간 (초)
    int distract_count = 0; // 이탈한 횟수
    int was_distract = 0;   // 바로 전에 이탈 중이었으면 1
    int key;
    int keyword_sec[MAX_KEYWORDS] = {0}; // 키워드마다 머무른 시간
    int index = 0; // 현재 걸린 키워드 인덱스 (없다면 -1)
    int i = 0; 

    SetConsoleOutputCP(CP_UTF8); // 한글을 출력
    SetConsoleCP(CP_UTF8); // 한글을 입력 
    load_keywords(); // 학습 이탈 키워드
    keyword_menu(); // 학습 이탈 키워드 추가 및 삭제 

    printf("!!기록 시작!! (끝내려면 창에서 Q를 누르세요) \n\n");

    while (1)
    {
        get_window_title(window_title);

        // 몇 번째 이탈 키워드에 걸렸는지 찾음 (없다면 -1)
        index = find_distract_index(window_title);

        if (index != -1)
        {
            distract_sec++;
            keyword_sec[index] ++; 

            // 학습 > 이탈로 바뀌었을 때 횟수 출력 
            if (was_distract == 0)
            {
                distract_count ++;
                printf("[이탈 시작]%s\n", window_title);
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

            // PgDn을 누르면 Q로 잘못 인식돼서 꺼짐
            // PgDn같은 특수 키는 무시함 
            if (key == 0 || key == 0xE0)
            {
                _getch();
            }
            
            else if (key == 'q' || key == 'Q')
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

    printf("\n[어플별 머무른 시간]\n");

    for (i = 0; i < distract_keyword_count; i++)
    {
        // 머문 시간이 0초보다 큰 어플만 출력함 
        if (keyword_sec[i] > 0)
        {
            printf("%s: ", distract_keywords[i]);
            print_time(keyword_sec[i]);
            printf("\n");
        }
    }

    // 이탈한 어플이 없을때 
    if (distract_sec == 0)
    {
        printf("학습에 집중하였습니다!\n");
    }

    printf("======================\n");

    return 0;
}
#endif // DETECT_STANDALONE