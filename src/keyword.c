#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "keyword.h"

#define DEFAULT_KEYWORD_COUNT 4

// 프로그램 시작 시 목록에 먼저 넣는 기본 키워드
static const char default_keywords[DEFAULT_KEYWORD_COUNT][KEYWORD_SIZE] = {
    "YouTube", "카카오톡", "Instagram", "Facebook"
};

// 현재 사용하는 키워드 목록과 개수
static char distract_keywords[MAX_KEYWORDS][KEYWORD_SIZE];
static int distract_keyword_count;

// 입력한 키워드의 칸 번호를 찾음
// 목록에 없으면 -1을 반환함
static int find_keyword(const char *word)
{
    for (int i = 0; i < distract_keyword_count; i++)
        if (strcmp(distract_keywords[i], word) == 0)
            return i;

    return -1;
}

// 기본 키워드인지 확인함
static int is_default_keyword(const char *word)
{
    for (int i = 0; i < DEFAULT_KEYWORD_COUNT; i++)
        if (strcmp(default_keywords[i], word) == 0)
            return 1;

    return 0;
}

// 현재 저장된 키워드의 개수를 반환함
int keyword_count(void)
{
    return distract_keyword_count;
}

// 입력받은 번호에 해당하는 키워드를 반환함
const char *keyword_at(int index)
{
    if (index < 0 || index >= distract_keyword_count)
        return NULL;

    return distract_keywords[index];
}

// 새로운 키워드를 목록에 추가함
int add_keyword(const char *word)
{
    // 빈 입력, 길이 초과, 줄바꿈, 중복, 개수 초과 검사
    if (!word || !word[0] ||
        strlen(word) >= KEYWORD_SIZE ||
        strpbrk(word, "\r\n") ||
        distract_keyword_count >= MAX_KEYWORDS ||
        find_keyword(word) >= 0)
        return 0;

    // 마지막 빈칸에 키워드를 저장하고 개수를 증가시킴
    strcpy(distract_keywords[distract_keyword_count++], word);

    return 1;
}

// 입력받은 키워드를 목록에서 삭제함
int remove_keyword(const char *word)
{
    if (!word) return 0;

    int index = find_keyword(word);
    if (index < 0) return 0;

    // 삭제한 위치부터 뒤의 키워드를 한 칸씩 앞으로 이동함
    for (int i = index; i < distract_keyword_count - 1; i++)
        strcpy(distract_keywords[i], distract_keywords[i + 1]);

    distract_keyword_count--;

    return 1;
}

// 기본 키워드와 파일에 저장된 키워드를 불러옴
void load_keywords(void)
{
    distract_keyword_count = 0;

    // 기본 키워드를 먼저 추가함
    for (int i = 0; i < DEFAULT_KEYWORD_COUNT; i++)
        add_keyword(default_keywords[i]);

    FILE *file = fopen(FILE_DISTRACT_KEYWORDS, "r");

    // 파일이 없으면 기본 키워드만 사용함
    if (!file) return;

    char line[KEYWORD_SIZE + 1];

    // 파일에서 키워드를 한 줄씩 읽음
    while (distract_keyword_count < MAX_KEYWORDS &&
           fgets(line, sizeof(line), file))
    {
        size_t end = strcspn(line, "\r\n");

        // 너무 긴 줄은 나머지를 읽어 버리고 넘어감
        if (line[end] == '\0' && end >= KEYWORD_SIZE)
        {
            int ch;
            while ((ch = fgetc(file)) != '\n' && ch != EOF);
            continue;
        }

        // 줄바꿈을 제거하고 키워드를 추가함
        line[end] = '\0';
        add_keyword(line);
    }

    fclose(file);
}

// 사용자 키워드를 파일에 저장함
int save_keywords(void)
{
    FILE *file = fopen(FILE_DISTRACT_KEYWORDS, "w");

    if (!file) return 0;

    for (int i = 0; i < distract_keyword_count; i++)
    {
        const char *word = distract_keywords[i];

        // 기본 키워드를 제외한 키워드만 저장함
        if (!is_default_keyword(word) &&
            fprintf(file, "%s\n", word) < 0)
        {
            fclose(file);
            return 0;
        }
    }

    return fclose(file) == 0;
}

// 콘솔에서 한 줄을 읽고 줄바꿈을 제거함
// 성공하면 1, 길이 초과는 0, 입력 종료는 -1을 반환함
static int read_menu_line(char text[], int size)
{
    if (!fgets(text, size, stdin)) return -1;

    size_t end = strcspn(text, "\r\n");

    // 입력이 너무 길면 남은 입력을 제거함
    if (text[end] == '\0' && !feof(stdin))
    {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF);
        return 0;
    }

    text[end] = '\0';
    return 1;
}

// 키워드 설정 메뉴
void keyword(void)
{
    char choice[8], word[KEYWORD_SIZE + 1];

    while (1)
    {
        system("cls");

        printf("========================================\n");
        printf("         0:Tracker - 키워드 수정\n");
        printf("========================================\n");
        printf("\n[이탈 키워드 목록]\n");

        // 현재 등록된 키워드를 순서대로 출력함
        for (int i = 0; i < keyword_count(); i++)
            printf(" %d. %s\n", i + 1, keyword_at(i));

        printf("\n[1] 추가  [2] 삭제  [0] 돌아가기\n");
        printf("\n========================================\n\n");
        printf("선택>> ");

        int result = read_menu_line(choice, sizeof(choice));

        // 0을 입력하거나 입력이 종료되면 메뉴에서 나감
        if (result < 0) return;
        if (result == 1 && strcmp(choice, "0") == 0) return;

        // 잘못된 메뉴 번호를 검사함
        if (!result || (strcmp(choice, "1") != 0 &&
                        strcmp(choice, "2") != 0))
            printf("0, 1, 2 중에서 선택해 주세요.\n");

        else
        {
            // 1이면 추가, 2이면 삭제
            int adding = strcmp(choice, "1") == 0;

            printf("%s할 키워드>> ", adding ? "추가" : "삭제");
            result = read_menu_line(word, sizeof(word));

            if (result < 0) return;

            // 키워드 입력이 올바른지 검사함
            if (!result || !word[0] ||
                strlen(word) >= KEYWORD_SIZE)
                printf("키워드가 비어 있거나 너무 깁니다.\n");

            // 키워드를 추가하거나 삭제함
            else if (!(adding ? add_keyword(word) :
                                remove_keyword(word)))
                printf("키워드 %s에 실패했습니다.\n",
                    adding ? "추가" : "삭제");

            // 변경된 목록을 파일에 저장함
            else if (!save_keywords())
                printf("목록은 변경됐지만 파일 저장에 실패했습니다.\n");

            else if (!adding && is_default_keyword(word))
                printf("삭제했습니다. 기본 키워드는 재실행 시 복구됩니다.\n");

            else
                printf("%s했습니다.\n", adding ? "추가" : "삭제");
        }

        // 안내 메시지가 바로 지워지지 않도록 대기함
        printf("\nEnter 키를 누르세요.");
        if (read_menu_line(choice, sizeof(choice)) < 0)
            return;
    }
}