#ifndef DETECT_H
#define DETECT_H

#include "common.h"

#define DEFAULT_KEYWORD_COUNT 4 // 기본 이탈 키워드 개수 

void get_window_title(char window_title[]);
int is_distract_title(char window_title[]);

int has_keyword(char word[]);
void load_keywords(void);
void save_keywords(void);


#endif
