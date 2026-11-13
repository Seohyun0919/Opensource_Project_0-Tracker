
#include <stdio.h>
#include <string.h>
#include "timer.h"

#define LINE_BUF_SIZE 256

// 문자열의 글자 수를 계산하는 함수
static int count_chars(const char *str)
{
    int count = 0;
    int i;

    for (i = 0; str[i] != '\0'; i++) {
        // UTF-8 이어지는 바이트는 글자 수에서 제외
        if (((unsigned char)str[i] & 0xC0) != 0x80) {
            count++;
        }
    }

    return count;
}

// 한 줄을 입력받는 함수
// 성공: 1, 입력 실패: 0, 입력 길이 초과: -1
static int read_line(char buf[], int size)
{
    int ch;

    if (fgets(buf, size, stdin) == NULL) {
        return 0;
    }

    // 입력한 문자열에 줄바꿈이 있는지 확인
    if (strchr(buf, '\n') == NULL && !feof(stdin)) {

        // 버퍼에 남아 있는 입력 내용 제거
        while ((ch = getchar()) != '\n' && ch != EOF) {
        }

        return -1;
    }

    // 줄바꿈 문자 제거
    buf[strcspn(buf, "\r\n")] = '\0';

    return 1;
}

// 이름이나 과목명을 입력받는 함수
static void input_text(const char *prompt, char out[], int max_chars)
{
    char line[LINE_BUF_SIZE];
    int len;
    int result;

    while (1) {
        printf("%s (1~%d자): ", prompt, max_chars);

        result = read_line(line, LINE_BUF_SIZE);

        // 입력 실패 시 함수 종료
        if (result == 0) {
            out[0] = '\0';
            return;
        }

        // 입력 길이가 너무 긴 경우
        if (result == -1) {
            printf("입력이 너무 깁니다. 다시 입력하세요.\n");
            continue;
        }

        // 입력한 글자 수 계산
        len = count_chars(line);

        // 빈칸 검사
        if (len == 0) {
            printf("빈칸은 입력할 수 없습니다.\n");
        }

        // 최대 글자 수 검사
        else if (len > max_chars) {
            printf("%d자를 넘을 수 없습니다.\n", max_chars);
        }

        // 배열에 저장할 수 있는 바이트 크기 검사
        else if (strlen(line) > (size_t)max_chars * 3) {
            printf("지원하지 않는 문자가 있습니다.\n");
        }

        // 모든 조건을 만족하면 문자열 저장
        else {
            strcpy(out, line);
            return;
        }
    }
}

// 사용자 이름 입력
void input_user_name(char user_name[])
{
    input_text("이름", user_name, MAX_NAME_LEN);
}

// 과목명 입력
void input_subject(char subject[])
{
    input_text("과목명", subject, MAX_SUBJECT_LEN);
}
