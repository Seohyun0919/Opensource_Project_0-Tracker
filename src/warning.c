
#include <windows.h>
#include "warning.h"

#pragma comment(lib, "user32.lib")
static volatile LONG warning_open = 0;

static DWORD WINAPI warningThread(LPVOID arg) {
    MessageBoxW(
        NULL,
        L"학습 이탈이 감지되었습니다!\n"
        L"공부에 다시 집중하세요.",
        L"0:Tracker WARNING",
        MB_OK | MB_ICONWARNING | MB_TOPMOST
    );

    InterlockedExchange(&warning_open, 0);
    return 0;
}

void warning(void) {
    HANDLE thread;

    // 경고창 중복 생성 방지
    if (InterlockedCompareExchange(
        &warning_open, 1, 0) != 0)
        return;

    thread = CreateThread(
        NULL, 0, warningThread,
        NULL, 0, NULL
    );

    if (thread)
        CloseHandle(thread);
    else
        InterlockedExchange(&warning_open, 0);
}