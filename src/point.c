#include <stdio.h>
#include "common.h"
#include "point.h"

int point(int study_sec, int distract_sec)
{
    int study_points = earn(study_sec);
    int distract_points = penalty(distract_sec);
    int total_points = loadPoints();

    if (total_points < 0)
        return -1;

    total_points += study_points - distract_points;

    if (total_points < 0)
        total_points = 0;

    FILE *fp = fopen(FILE_POINTS, "w");

    if (fp == NULL){
        printf("파일 열기 실패\n");
        return -1;
    }


    if (fprintf(fp, "%d\n", total_points) < 0) {
        printf("포인트 저장 실패 1\n");
        fclose(fp);     
        return -1;  
    }

    if (fclose(fp) != 0){
        printf("포인트 저장 실패 2\n");
        return -1;
    }
        

    return total_points;
}

int earn(int study_sec)
{
    return study_sec / 60 * 10;
}

int penalty(int distract_sec)
{
    return distract_sec / 60 * 10;
}
