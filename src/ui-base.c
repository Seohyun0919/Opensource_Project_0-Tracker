
#include <stdio.h>
#include <stdlib.h>

int main() {
    int menu;

    while (1) {
        system("cls");

        printf("========================================\n");
        printf("               0:Tracker\n");
        printf("========================================\n\n");
        printf("[1] 공부 시작\n\n");
        printf("[2] 공부 기록\n\n");
        printf("[3] Level\n\n");
        printf("[4] 설정\n\n");
        printf("[0] Exit\n\n");
        printf("----------------------------------------\n\n");
        printf("Select Menu >> ");

        if (scanf_s("%d", &menu) != 1) {
            while (getchar() != '\n');
            continue;
        }

        if (menu == 0) {
            printf("\n  Goodbye! Stay focused!\n");
            break;
        }

        if (menu >= 1 && menu <= 4)
            printf("\n  Menu %d selected.\n", menu);
        else
            printf("\n  Invalid selection. Try again.\n");

        printf("\n  Press Enter to return...");
        while (getchar() != '\n');
        getchar();
    }

    return 0;
}
