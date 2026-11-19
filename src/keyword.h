#ifndef KEYWORD_H
#define KEYWORD_H

#include "common.h"

// UTF-8 키워드를 담는 배열 크기. 마지막 한 칸은 '\0'에 사용함
#define KEYWORD_SIZE (MAX_KEYWORD_LEN * 3 + 1)

// 목록의 개수와 각 칸에 들어 있는 키워드를 읽음
int keyword_count(void);
const char *keyword_at(int index);

// 목록에 키워드를 추가하거나 삭제함. 성공하면 1, 실패하면 0
int add_keyword(const char *word);
int remove_keyword(const char *word);

// 기본 키워드와 저장된 키워드를 불러옴
void load_keywords(void);

// 사용자가 추가한 키워드를 파일에 저장함. 성공하면 1, 실패하면 0
int save_keywords(void);

// 메인 화면에서 4번을 선택하면 여는 키워드 설정 메뉴
void keyword(void);

#endif