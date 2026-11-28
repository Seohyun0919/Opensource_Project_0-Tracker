
#include <windows.h>
#include <objbase.h>
#include "warning.h"

// 경고창 크기
#define WARNING_WIDTH 660
#define WARNING_HEIGHT 600

// 경고창 중복 생성 방지
static volatile LONG warning_open = 0;

// GDI+ 이미지 자료형
typedef void GpImage;
typedef void GpGraphics;

typedef struct {
    UINT32 version;
    void* callback;
    BOOL suppressBackgroundThread;
    BOOL suppressExternalCodecs;
} GDIPlusInput;

// GDI+ 함수 선언
extern int WINAPI GdiplusStartup(
    ULONG_PTR*, const GDIPlusInput*, void*
);

extern void WINAPI GdiplusShutdown(
    ULONG_PTR
);

extern int WINAPI GdipLoadImageFromFile(
    const WCHAR*, GpImage**
);

extern int WINAPI GdipCreateFromHDC(
    HDC, GpGraphics**
);

extern int WINAPI GdipDrawImageRectI(
    GpGraphics*, GpImage*, INT, INT, INT, INT
);

extern int WINAPI GdipDeleteGraphics(
    GpGraphics*
);

extern int WINAPI GdipDisposeImage(
    GpImage*
);

// 경고창 관련 변수
static ULONG_PTR gdiplus_token = 0;
static GpImage* angry_image = NULL;

static HWND warning_window = NULL;
static BOOL button_hover = FALSE;

// 색상 설정
#define BG_COLOR RGB(250, 248, 246)
#define PRIMARY_COLOR RGB(152, 49, 81)
#define DARK_COLOR RGB(112, 35, 60)
#define MUTED_COLOR RGB(134, 119, 125)

// 버튼 위치
static RECT button_rect = {
    36, 516, 624, 576
};

// 가운데 정렬 텍스트 출력
static void warning_text(
    HDC hdc,
    const wchar_t* message,
    int y,
    int size,
    COLORREF color
) {
    HFONT font = CreateFontW(
        -size, 0, 0, 0,
        FW_BOLD,
        FALSE, FALSE, FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY,
        DEFAULT_PITCH,
        L"맑은 고딕"
    );

    HFONT old_font = SelectObject(hdc, font);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, color);

    RECT rect = {
        0, y, WARNING_WIDTH, y + size + 10
    };

    DrawTextW(
        hdc, message, -1, &rect,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE
    );

    SelectObject(hdc, old_font);
    DeleteObject(font);
}

// 경고창 화면 출력
static void draw_warning(HWND hwnd, HDC hdc) {

    RECT client;
    GetClientRect(hwnd, &client);

    // 배경
    HBRUSH background = CreateSolidBrush(BG_COLOR);

    FillRect(hdc, &client, background);

    DeleteObject(background);

    // 제목
    warning_text(
        hdc,
        L"딴짓 금지!",
        30, 54,
        PRIMARY_COLOR
    );

    // 화난 우왕이 이미지
    if (angry_image != NULL) {

        GpGraphics* graphics = NULL;

        if (GdipCreateFromHDC(hdc, &graphics) == 0) {

            GdipDrawImageRectI(
                graphics,
                angry_image,
                150, 100,
                360, 360
            );

            GdipDeleteGraphics(graphics);
        }
    }
    else {

        // 이미지 로딩 실패 시 대체 표시
        warning_text(
            hdc,
            L"!",
            125, 100,
            PRIMARY_COLOR
        );
    }

    // 버튼 배경
    HBRUSH button_brush = CreateSolidBrush(
        button_hover ? DARK_COLOR : PRIMARY_COLOR
    );

    HBRUSH old_brush =
        SelectObject(hdc, button_brush);

    HPEN old_pen =
        SelectObject(hdc, GetStockObject(NULL_PEN));

    RoundRect(
        hdc,
        button_rect.left,
        button_rect.top,
        button_rect.right,
        button_rect.bottom,
        15, 15
    );

    SelectObject(hdc, old_brush);
    SelectObject(hdc, old_pen);

    DeleteObject(button_brush);

    // 버튼 글자
    warning_text(
        hdc,
        L"공부하러 가기",
        525, 32,
        RGB(255, 255, 255)
    );
}

// 경고창 이벤트 처리
static LRESULT CALLBACK WarningWndProc(
    HWND hwnd,
    UINT msg,
    WPARAM wp,
    LPARAM lp
) {
    switch (msg) {

        case WM_PAINT: {

            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            draw_warning(hwnd, hdc);

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_ERASEBKGND:
            return 1;

        case WM_MOUSEMOVE: {

            POINT p = {
                (short)LOWORD(lp),
                (short)HIWORD(lp)
            };

            BOOL hover = PtInRect(&button_rect, p);

            if (hover != button_hover) {

                button_hover = hover;

                InvalidateRect(hwnd, NULL, FALSE);
            }

            TRACKMOUSEEVENT track = {0};

            track.cbSize = sizeof(track);
            track.dwFlags = TME_LEAVE;
            track.hwndTrack = hwnd;

            TrackMouseEvent(&track);

            SetCursor(
                LoadCursorA(
                    NULL,
                    hover ? IDC_HAND : IDC_ARROW
                )
            );

            return 0;
        }

        case WM_MOUSELEAVE:

            button_hover = FALSE;

            InvalidateRect(hwnd, NULL, FALSE);

            return 0;

        case WM_LBUTTONUP: {

            POINT p = {
                (short)LOWORD(lp),
                (short)HIWORD(lp)
            };

            if (PtInRect(&button_rect, p)) {

                DestroyWindow(hwnd);
            }

            return 0;
        }

        case WM_KEYDOWN:

            if (wp == VK_RETURN || wp == VK_ESCAPE) {

                DestroyWindow(hwnd);
            }

            return 0;

        case WM_CLOSE:

            DestroyWindow(hwnd);

            return 0;

        case WM_DESTROY:

            warning_window = NULL;

            PostQuitMessage(0);

            return 0;
    }

    return DefWindowProcW(hwnd, msg, wp, lp);
}

// 기존 경고음
static DWORD WINAPI soundThread(LPVOID arg) {

    (void)arg;

    Beep(1500, 300);
    Beep(1000, 300);

    Beep(1500, 300);
    Beep(1000, 300);

    Beep(1500, 300);
    Beep(1000, 300);

    return 0;
}

// 경고창 실행 스레드
static DWORD WINAPI warningThread(LPVOID arg) {

    (void)arg;

    HINSTANCE instance = GetModuleHandleW(NULL);

    // GDI+ 초기화
    GDIPlusInput input = {0};

    input.version = 1;

    if (GdiplusStartup(
        &gdiplus_token, &input, NULL
    ) == 0) {

        GdipLoadImageFromFile(
            L"assets/uwang_angry.png",
            &angry_image
        );
    }

    // 기존 전체 화면 붉은색 오버레이
    HBRUSH brush = CreateSolidBrush(
        RGB(120, 0, 0)
    );

    WNDCLASSW overlay_class = {0};

    overlay_class.lpfnWndProc = DefWindowProcW;
    overlay_class.hInstance = instance;
    overlay_class.hbrBackground = brush;
    overlay_class.lpszClassName = L"TrackerOverlay";

    RegisterClassW(&overlay_class);

    HWND overlay = CreateWindowExW(
        WS_EX_TOPMOST |
        WS_EX_LAYERED |
        WS_EX_TOOLWINDOW,

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

        SetLayeredWindowAttributes(
            overlay, 0, 220, LWA_ALPHA
        );

        ShowWindow(overlay, SW_SHOW);

        UpdateWindow(overlay);
    }

    // 커스텀 경고창 클래스
    WNDCLASSW warning_class = {0};

    warning_class.lpfnWndProc = WarningWndProc;
    warning_class.hInstance = instance;
    warning_class.hCursor = LoadCursorA(
        NULL, IDC_ARROW
    );

    warning_class.lpszClassName =
        L"TrackerCustomWarning";

    RegisterClassW(&warning_class);

    // 주 모니터 중앙 위치
    int x = (
        GetSystemMetrics(SM_CXSCREEN)
        - WARNING_WIDTH
    ) / 2;

    int y = (
        GetSystemMetrics(SM_CYSCREEN)
        - WARNING_HEIGHT
    ) / 2;

    button_hover = FALSE;

    // 커스텀 경고창 생성
    warning_window = CreateWindowExW(

        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,

        L"TrackerCustomWarning",

        L"0:Tracker WARNING",

        WS_POPUP,

        x, y,

        WARNING_WIDTH,
        WARNING_HEIGHT,

        overlay,
        NULL,
        instance,
        NULL
    );

    if (warning_window) {

        // 경고창 모서리를 둥글게 처리
        HRGN region = CreateRoundRectRgn(
            0, 0,
            WARNING_WIDTH + 1,
            WARNING_HEIGHT + 1,
            38, 38
        );

        if (region &&
            !SetWindowRgn(warning_window, region, TRUE)) {

            DeleteObject(region);
        }

        ShowWindow(warning_window, SW_SHOW);

        SetForegroundWindow(warning_window);

        UpdateWindow(warning_window);
    }

    // 경고음 별도 실행
    HANDLE sound = CreateThread(
        NULL, 0,
        soundThread,
        NULL, 0, NULL
    );

    // 경고창 이벤트 처리
    if (warning_window) {

        MSG msg;

        while (GetMessageW(&msg, NULL, 0, 0) > 0) {

            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
    else {

        // 커스텀 경고창 생성 실패 시
        MessageBoxW(
            overlay,
            L"학습 이탈이 감지되었습니다!\n"
            L"공부에 다시 집중하세요.",
            L"0:Tracker WARNING",
            MB_OK | MB_ICONWARNING |
            MB_TOPMOST | MB_SETFOREGROUND
        );
    }

    // 경고창 및 오버레이 제거
    if (overlay) {

        DestroyWindow(overlay);
    }

    UnregisterClassW(
        L"TrackerCustomWarning", instance
    );

    UnregisterClassW(
        L"TrackerOverlay", instance
    );

    DeleteObject(brush);

    // 이미지 메모리 해제
    if (angry_image) {

        GdipDisposeImage(angry_image);

        angry_image = NULL;
    }

    // GDI+ 종료
    if (gdiplus_token) {

        GdiplusShutdown(gdiplus_token);

        gdiplus_token = 0;
    }

    // 경고음 스레드 정리
    if (sound) {

        WaitForSingleObject(sound, INFINITE);

        CloseHandle(sound);
    }

    // 경고창 중복 생성 상태 초기화
    InterlockedExchange(&warning_open, 0);

    return 0;
}

// 외부에서 호출하는 기존 경고 함수
void warning(void) {

    HANDLE thread;

    // 경고창 중복 생성 방지
    if (InterlockedCompareExchange(
        &warning_open, 1, 0
    ) != 0) {

        return;
    }

    thread = CreateThread(
        NULL, 0,
        warningThread,
        NULL, 0, NULL
    );

    if (thread) {

        CloseHandle(thread);
    }
    else {

        InterlockedExchange(&warning_open, 0);
    }
}
