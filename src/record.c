// M3 기록 파일. 세션(M1)과 창 전환(M2)은 파일 끝에 한 줄씩 붙이고
// 키워드, 과목 목록은 통째로 덮어씀. 칸 순서는 RULES.md 파일 형식 그대로
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

#define SESSION_FIELDS 9
// 세션 줄은 모든 칸을 최대로 채워도 210바이트 안쪽. 키워드, 과목 줄은 더 짧음
// 주의: 창 전환 줄은 제목만 768바이트라 읽을 때는 더 큰 버퍼가 필요함
#define LINE_BUF_SIZE 512
#define KEYWORD_BUF (MAX_KEYWORD_LEN * 3 + 1)
#define SUBJECT_BUF (MAX_SUBJECT_LEN * 3 + 1)

// keyword.c의 기본 이탈 키워드와 같게 맞춤
static const char default_distract[][KEYWORD_BUF] = {
    "YouTube", "카카오톡", "Instagram", "Facebook"
};
static const char default_allow[][KEYWORD_BUF] = { "강의", "인강" };

// 문자열 칸에 탭, 줄바꿈이 섞이면 칸이 밀려서 그 줄을 못 읽게 됨
// 창 제목은 프로그램마다 달라서 실제로 탭이 들어 있을 수 있음
static void writeField(FILE* fp, const char* text)
{
    for (; *text != '\0'; text++) {
        fputc(strchr("\t\r\n", *text) ? ' ' : *text, fp);
    }
}

// 디스크가 꽉 차는 등 쓰기가 실패해도 열기는 성공하므로 따로 확인함
static int finishWrite(FILE* fp)
{
    int ok = !ferror(fp);

    if (fclose(fp) != 0) {
        ok = 0;
    }
    return ok;
}

// fgets가 버퍼에서 끊은 줄이면 나머지를 버리고 1. 안 버리면 뒷부분이 다음 줄로 읽힘
static int skipLongLine(FILE* fp, const char* line)
{
    int c;

    if (strchr(line, '\n') != NULL || feof(fp)) {
        return 0;
    }
    while ((c = fgetc(fp)) != '\n' && c != EOF) {
    }
    return 1;
}

// 칸 수가 안 맞는 줄(쓰다가 끊긴 줄 등)은 0을 돌려서 건너뛰게 함
static int parseSession(char* line, Session* s)
{
    char* fields[SESSION_FIELDS];
    char* next = NULL;
    char* tok;
    int n = 0;

    line[strcspn(line, "\r\n")] = '\0';
    for (tok = strtok_s(line, "\t", &next); tok != NULL; tok = strtok_s(NULL, "\t", &next)) {
        if (n == SESSION_FIELDS) {
            return 0;
        }
        fields[n++] = tok;
    }
    if (n != SESSION_FIELDS) {
        return 0;
    }

    s->session_num = atoi(fields[0]);
    strncpy_s(s->user_name, sizeof(s->user_name), fields[1], _TRUNCATE);
    strncpy_s(s->date, sizeof(s->date), fields[2], _TRUNCATE);
    strncpy_s(s->subject, sizeof(s->subject), fields[3], _TRUNCATE);
    s->goal_min = atoi(fields[4]);
    s->study_sec = atoi(fields[5]);
    s->distract_sec = atoi(fields[6]);
    s->distract_count = atoi(fields[7]);
    s->goal_achieved = atoi(fields[8]);
    return 1;
}

// 줄 수 + 1로 하면 깨진 줄이 있을 때 번호가 겹쳐서, 가장 큰 번호 + 1로 함
int getNextSessionNum(void)
{
    FILE* fp;
    char line[LINE_BUF_SIZE];
    int num;
    int max_num = 0;

    if (fopen_s(&fp, FILE_SESSIONS, "r") != 0) {
        return 1;
    }
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (skipLongLine(fp, line)) {
            continue;
        }
        if (sscanf_s(line, "%d", &num) == 1 && num > max_num) {
            max_num = num;
        }
    }
    fclose(fp);
    return max_num + 1;
}

int saveSession(const Session* s)
{
    FILE* fp;

    if (fopen_s(&fp, FILE_SESSIONS, "a") != 0) {
        return 0;
    }
    fprintf(fp, "%d\t", s->session_num);
    writeField(fp, s->user_name);
    fprintf(fp, "\t%s\t", s->date);
    writeField(fp, s->subject);
    fprintf(fp, "\t%d\t%d\t%d\t%d\t%d\n", s->goal_min, s->study_sec,
            s->distract_sec, s->distract_count, s->goal_achieved);
    return finishWrite(fp);
}

int saveWindowLog(int session_num, const WindowLog* log)
{
    FILE* fp;

    if (fopen_s(&fp, FILE_WINDOW_LOG, "a") != 0) {
        return 0;
    }
    fprintf(fp, "%d\t%s\t", session_num, log->switch_time);
    writeField(fp, log->window_title);
    fprintf(fp, "\t%d\t%d\n", log->stay_sec, log->is_distract);
    return finishWrite(fp);
}

int loadSessions(Session* list, int max)
{
    FILE* fp;
    char line[LINE_BUF_SIZE];
    Session s;
    int count = 0;

    // 아직 한 번도 저장 안 했으면 파일이 없음. 기록 0개로 봄
    if (fopen_s(&fp, FILE_SESSIONS, "r") != 0) {
        return 0;
    }
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (skipLongLine(fp, line) || !parseSession(line, &s)) {
            continue;
        }
        if (count < max) {
            list[count] = s;
        }
        count++;
    }
    fclose(fp);
    return count;
}

// 키워드와 과목은 배열 한 칸 크기만 달라서 width로 받아 같이 씀
static int writeLines(const char* file, const char* list, size_t width, int count)
{
    FILE* fp;
    int i;

    if (fopen_s(&fp, file, "w") != 0) {
        return 0;
    }
    for (i = 0; i < count; i++) {
        writeField(fp, list + i * width);
        fputc('\n', fp);
    }
    return finishWrite(fp);
}

// 파일이 없으면 -1. 빈 파일(0개)과 구분해야 기본값을 넣을지 정할 수 있음
static int readLines(const char* file, char* list, size_t width, int max)
{
    FILE* fp;
    char line[LINE_BUF_SIZE];
    int count = 0;

    if (fopen_s(&fp, file, "r") != 0) {
        return -1;
    }
    while (count < max && fgets(line, sizeof(line), fp) != NULL) {
        if (skipLongLine(fp, line)) {
            continue;
        }
        line[strcspn(line, "\r\n")] = '\0';
        // 빈 줄은 메모장으로 고치다 생기기 쉬움. 너무 긴 줄은 자르면 한글이 깨져서 버림
        if (line[0] == '\0' || strlen(line) >= width) {
            continue;
        }
        strcpy_s(list + count * width, width, line);
        count++;
    }
    fclose(fp);
    return count;
}

int saveKeywords(const char* file, char list[][KEYWORD_BUF], int count)
{
    return writeLines(file, list[0], KEYWORD_BUF, count);
}

int loadKeywords(const char* file, char list[][KEYWORD_BUF], int max)
{
    const char (*defaults)[KEYWORD_BUF] = NULL;
    int default_count = 0;
    int count = readLines(file, list[0], KEYWORD_BUF, max);

    if (count < 0) {
        count = 0;
        if (strcmp(file, FILE_DISTRACT_KEYWORDS) == 0) {
            defaults = default_distract;
            default_count = sizeof(default_distract) / sizeof(default_distract[0]);
        } else if (strcmp(file, FILE_ALLOW_KEYWORDS) == 0) {
            defaults = default_allow;
            default_count = sizeof(default_allow) / sizeof(default_allow[0]);
        }
        for (; count < default_count && count < max; count++) {
            strcpy_s(list[count], KEYWORD_BUF, defaults[count]);
        }
        // 파일을 못 만들어도 기본값으로 감지는 돌아가야 해서 실패를 무시함
        saveKeywords(file, list, count);
    }
    return count;
}

// 과목은 기본값이 없음. 파일이 없으면 0개이고, 처음 저장할 때 파일이 생김
int loadSubjects(char list[][SUBJECT_BUF], int max)
{
    int count = readLines(FILE_SUBJECTS, list[0], SUBJECT_BUF, max);

    return count < 0 ? 0 : count;
}

int saveSubjects(char list[][SUBJECT_BUF], int count)
{
    return writeLines(FILE_SUBJECTS, list[0], SUBJECT_BUF, count);
}
