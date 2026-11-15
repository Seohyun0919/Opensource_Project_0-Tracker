#ifndef DETECT_H
#define DETECT_H

#define MAX_TITLE_LEN 1024 //창 제목의 최대 길이임 (바이트)
#define MAX_KEYWORDS 100 //키워드 최대 개수임
#define MAX_KEYWORD_LEN 100 //키워드 최대 길이임 (바이트)
#define FILE_DISTRACT_KEYWORDS "distract_keywords.txt" // 사용자가 추가하는 이탈 키워드 저장하는 파일
#define DEFAULT_KEYWORD_COUNT 4 // 기본 이탈 키워드 개수 

void get_window_title(char window_title[]);
int is_distract_title(char window_title[]);

int has_keyword(char word[]);
void load_keywords(void);
void save_keywords(void);


#endif
