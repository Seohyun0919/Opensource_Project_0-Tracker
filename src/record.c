// M3 기록 저장. M1이 끝난 세션을 넘기면 sessions.txt 끝에 한 줄씩 붙임
// M4~M6은 loadSessions로 읽어 감. 칸 순서는 RULES.md 파일 형식 그대로
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

#define SESSION_FIELDS 9
// 모든 칸을 최대로 채워도 한 줄이 210바이트를 안 넘어서 넉넉히 잡음
#define LINE_BUF_SIZE 512

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
        if (!parseSession(line, &s)) {
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
