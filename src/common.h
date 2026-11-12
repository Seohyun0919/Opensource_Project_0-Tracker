// M3 공통 구조체, 파일 이름, 모듈 간 함수 원형
// M1, M2가 기록을 넘기고 M4~M7이 조회 결과를 받아 감
#ifndef COMMON_H
#define COMMON_H

// 길이는 글자 수. 배열은 UTF-8 한글 3바이트 기준으로 잡음
#define MAX_NAME_LEN        20
#define MAX_SUBJECT_LEN     20
#define MAX_TITLE_LEN       256
#define MAX_KEYWORDS        20
#define MAX_KEYWORD_LEN     30
#define DISTRACT_GRACE_SEC  5

#define FILE_SESSIONS           "sessions.txt"
#define FILE_WINDOW_LOG         "window_log.txt"
#define FILE_DISTRACT_KEYWORDS  "distract_keywords.txt"
#define FILE_ALLOW_KEYWORDS     "allow_keywords.txt"
#define FILE_SUBJECTS           "subjects.txt"
#define FILE_POINTS             "points.txt"
#define FILE_TEMP               "temp_session.txt"

typedef struct {
    int session_num;
    char user_name[MAX_NAME_LEN * 3 + 1];
    // YYYY-MM-DD
    char date[11];
    char subject[MAX_SUBJECT_LEN * 3 + 1];
    int goal_min;
    int study_sec;
    int distract_sec;
    int distract_count;
    int goal_achieved;
} Session;

typedef struct {
    // HH:MM:SS
    char switch_time[9];
    char window_title[MAX_TITLE_LEN * 3 + 1];
    int stay_sec;
    int is_distract;
} WindowLog;

typedef struct {
    int total_study_sec;
    int total_distract_sec;
    int total_distract_count;
    double focus_rate;
    int avg_focus_sec;
    char top_distract_window[MAX_TITLE_LEN * 3 + 1];
} AnalysisResult;

// 저장 함수는 성공하면 1, 파일을 못 열면 0
int getNextSessionNum(void);
int saveSession(const Session* s);
int saveWindowLog(int session_num, const WindowLog* log);

// M1이 1분마다 덮어씀. 정상 종료하면 removeTempSession으로 지움
int saveTempSession(const Session* s);
// 임시 파일이 없으면 0. 시작할 때 복구 여부를 묻는 데 씀
int loadTempSession(Session* s);
void removeTempSession(void);

// 찾은 개수를 돌려주고 max개까지만 채움
int loadSessions(Session* list, int max);
int findSessionsByDate(const char* date, Session* list, int max);
int findSessionsBySubject(const char* subject, Session* list, int max);
int loadWindowLogs(int session_num, WindowLog* list, int max);

// 전날 기록이 없으면 -1 (M5 보너스 판정용)
int getPrevDayStudySec(const char* date);
// 같은 과목에서 study_sec이 가장 긴 세션. 없으면 0
int findBestSession(const char* subject, Session* best);

// 파일이 없으면 기본값으로 만든 뒤 읽음
int loadKeywords(const char* file, char list[][MAX_KEYWORD_LEN * 3 + 1], int max);
int saveKeywords(const char* file, char list[][MAX_KEYWORD_LEN * 3 + 1], int count);
int loadSubjects(char list[][MAX_SUBJECT_LEN * 3 + 1], int max);
int saveSubjects(char list[][MAX_SUBJECT_LEN * 3 + 1], int count);
// 파일이 없으면 0점
int loadPoints(void);
int savePoints(int total_points);

void analyzeSession(int session_num, AnalysisResult* result);
int getSubjectStudySec(const char* subject);
// 분마다 1(집중) 또는 0(이탈)을 채우고 채운 분 수를 돌려줌
int makeTimeline(int session_num, int* timeline, int max_min);

#endif
