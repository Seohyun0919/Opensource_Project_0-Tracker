
#include <windows.h>
#include "warning.h"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
static volatile LONG warning_open = 0;

static DWORD WINAPI warningThread(LPVOID arg) {
    HINSTANCE instance = GetModuleHandleW(NULL);
    HBRUSH brush = CreateSolidBrush(RGB(120, 0, 0));

    WNDCLASSW wc = {0};
    wc.lpfnWndProc = DefWindowProcW;
    wc.hInstance = instance;
    wc.hbrBackground = brush;
    wc.lpszClassName = L"TrackerOverlay";

    RegisterClassW(&wc);

    HWND overlay = CreateWindowExW(
    WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
    L"TrackerOverlay",
    L"0:Tracker WARNING",
    WS_POPUP,
    GetSystemMetrics(SM_XVIRTUALSCREEN),
    GetSystemMetrics(SM_YVIRTUALSCREEN),
    GetSystemMetrics(SM_CXVIRTUALSCREEN),
    GetSystemMetrics(SM_CYVIRTUALSCREEN),
    NULL, NULL, instance, NULL
    );
    
    if (overlay) { 
        SetLayeredWindowAttributes(overlay, 0, 220, LWA_ALPHA);    
        ShowWindow(overlay, SW_SHOW);
        UpdateWindow(overlay);
    }

    Beep(1500, 300);
    Beep(1000, 300);
    Beep(1500, 300);
    Beep(1000, 300);
    Beep(1500, 300);
    Beep(1000, 300);
    
    MessageBoxW(
        overlay,
        L"\n\n학습 이탈이 감지되었습니다!\n"
        L"\n공부에 다시 집중하세요.\n\n",
        L"0:Tracker WARNING\n",
        MB_OK | MB_ICONWARNING | MB_TOPMOST | MB_SETFOREGROUND
    );

    if (overlay){
        DestroyWindow(overlay);
    }
    
    UnregisterClassW(L"TrackerOverlay", instance);

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