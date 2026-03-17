#include <windows.h>
#include <spdlog/spdlog.h>
#include "context.h"

// 전역 변수 (DirectX 핵심 자원들)
ID3D11Device* g_device = nullptr;
ID3D11DeviceContext* g_context = nullptr;
IDXGISwapChain* g_swapChain = nullptr;
ID3D11RenderTargetView* g_renderTargetView = nullptr;
ContextUPtr g_appContext = nullptr;

// 윈도우 프로시저 (메시지 처리)
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

// DirectX 초기화 함수
bool InitDirectX(HWND hWnd) {
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 1;
    sd.BufferDesc.Width = 1280;
    sd.BufferDesc.Height = 720;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    UINT createDeviceFlags = 0;


    // 1. 디바이스 및 스왑 체인 생성
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags,
        nullptr, 0, D3D11_SDK_VERSION, &sd, &g_swapChain, &g_device, nullptr, &g_context
    );
    if (FAILED(hr)) return false;

    // 2. 렌더 타겟 뷰(도화지) 생성
    ID3D11Texture2D* pBackBuffer = nullptr;
    g_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
    g_device->CreateRenderTargetView(pBackBuffer, nullptr, &g_renderTargetView);
    pBackBuffer->Release();

    return true;
}

int main() {
    // 콘솔 디버깅 로그 레벨 설정 (필요시 활성화)
    spdlog::set_level(spdlog::level::debug);

    HINSTANCE hInstance = GetModuleHandle(NULL);
    const char* className = "DX11WindowClass";

    // 윈도우 클래스 등록
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, WindowProc, 0, 0, hInstance, NULL, LoadCursor(NULL, IDC_ARROW), NULL, NULL, className, NULL };
    RegisterClassEx(&wc);

    // 윈도우 창 생성
    HWND hWnd = CreateWindowEx(0, className, "DirectX 11 Triangle", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720, NULL, NULL, hInstance, NULL);
    ShowWindow(hWnd, SW_SHOW);

    // DirectX 초기화
    if (!InitDirectX(hWnd)) {
        SPDLOG_ERROR("DirectX Init Failed");
        return -1;
    }

    // Context (파이프라인 및 리소스) 초기화
    g_appContext = Context::Create(g_device);
    if (!g_appContext) {
        SPDLOG_ERROR("Context Create Failed");
        return -1;
    }

    // 메인 메시지 루프
    MSG msg = { 0 };
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            // ================== 렌더링 루프 시작 ==================
            
            // 1. 화면 지우기 (파란색 배경)
            float clearColor[4] = { 0.1f, 0.2f, 0.4f, 1.0f }; // 색상을 살짝 밝게 조절했습니다
            g_context->ClearRenderTargetView(g_renderTargetView, clearColor);

            // [★ 수정된 핵심 부분] 그리기 직전에 도화지와 뷰포트를 명시적으로 바인딩
            g_context->OMSetRenderTargets(1, &g_renderTargetView, nullptr);
            
            D3D11_VIEWPORT vp = {};
            vp.TopLeftX = 0.0f;
            vp.TopLeftY = 0.0f;
            vp.Width = 1280.0f;
            vp.Height = 720.0f;
            vp.MinDepth = 0.0f;
            vp.MaxDepth = 1.0f;
            g_context->RSSetViewports(1, &vp);

            // 2. 삼각형 그리기 (파이프라인 실행)
            g_appContext->Render(g_context);

            // 3. 화면 스왑 (Present)
            g_swapChain->Present(1, 0);
            
            // ======================================================
        }
    }

    // 종료 시 COM 객체 메모리 해제 등은 스마트 포인터와 ComPtr이 자동으로 처리합니다.
    return 0;
}