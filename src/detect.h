#ifndef DETECT_H
#define DETECT_H

#define MAX_TITLE_LEN 1024 //창 제목의 최대 길이임 (바이트)
#define MAX_KEYWORDS 100 //키워드 최대 개수임
#define MAX_KEYWORD_LEN 100 //키워드 최대 길이임 (바이트)

void get_window_title(char window_title[]);
int is_distract_title(char window_title[]);

#endif
