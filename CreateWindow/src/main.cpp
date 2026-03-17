#include <windows.h>
#include <spdlog/spdlog.h>

// 윈도우 메시지 처리 함수 (창 닫기, 크기 조절 등)
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

int main() {
    // 1. 현재 프로그램의 인스턴스 핸들을 직접 가져와야 합니다.
    HINSTANCE hInstance = GetModuleHandle(NULL);
    
    // 2. 로그 확인 (창이 안 뜨면 터미널에 에러 로그가 찍히는지 보세요)
    spdlog::info("Application Started");

    const char* className = "DX11WindowClass";
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, WindowProc, 0, 0, 
                      hInstance, NULL, LoadCursor(NULL, IDC_ARROW), NULL, NULL, className, NULL };
    
    if (!RegisterClassEx(&wc)) {
        spdlog::error("Window Class Registration Failed!");
        return -1;
    }

    // 3. 창 생성 (창 크기가 0이 아닌지, 이름이 맞는지 확인)
    HWND hWnd = CreateWindowEx(0, className, "Zenless Style PBR Pipeline", WS_OVERLAPPEDWINDOW,
                               CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720, NULL, NULL, hInstance, NULL);

    if (!hWnd) {
        spdlog::error("Window Creation Failed! Error Code: {}", GetLastError());
        return -1;
    }

    ShowWindow(hWnd, SW_SHOW); // SW_SHOW로 확실히 지정
    UpdateWindow(hWnd);        // 즉시 그리도록 명령

    spdlog::info("Window Displayed Successfully.");

    // 4. 메시지 루프 (여기가 멈춰있어야 창이 유지됩니다)
    MSG msg = { 0 };
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return 0;
}