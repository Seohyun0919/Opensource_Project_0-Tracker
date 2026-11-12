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

#endif
