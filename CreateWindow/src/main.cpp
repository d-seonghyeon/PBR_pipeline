#include <windows.h>
#include <spdlog/spdlog.h>
#include "context.h"

//// 전역 변수 (DirectX 핵심 자원들)
ID3D11Device* g_device = nullptr;
ID3D11DeviceContext* g_context = nullptr;
IDXGISwapChain* g_swapChain = nullptr;
ID3D11RenderTargetView* g_renderTargetView = nullptr;
ContextUPtr g_appContext = nullptr;
//HWND: 윈도우를 만들거나 메시지를 보낼때 사용하는 식별자

uint32_t g_width = WINDOW_WIDTH;
uint32_t g_height = WINDOW_HEIGHT;

void RenderFrame();

void Resize(UINT width, UINT height)
{
    // 1. 장치가 아직 없거나, 창이 최소화되었을 때 실행x
    if (!g_device || !g_context || !g_swapChain)
        return;

    if (width == 0 || height == 0)
        return;

    // 2. 기존 RTV 해제 (렌더 타겟을 물고 있으면 스왑체인 리사이즈가 실패함)
    g_context->OMSetRenderTargets(0, nullptr, nullptr);

    if (g_renderTargetView)
    {
        g_renderTargetView->Release();
        g_renderTargetView = nullptr;
    }

    // 3. SwapChain 버퍼 리사이즈
    HRESULT hr = g_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
    if (FAILED(hr)) {
        SPDLOG_ERROR("ResizeBuffers Failed!");
        return;
    }

    // 4. 새 백버퍼 가져오기 및 RTV 재생성
    ID3D11Texture2D* backBuffer = nullptr;
    g_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);

    g_device->CreateRenderTargetView(backBuffer, nullptr, &g_renderTargetView);
    backBuffer->Release();
    
    SPDLOG_INFO("Window Resized: {}x{}", width, height);
}


// 윈도우 프로시저 (메시지 처리)
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    
    switch (message)
    {
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }
        case WM_SIZE:
        {
            g_width = LOWORD(lParam);
            g_height = HIWORD(lParam);

            if(g_device){
                Resize(g_width, g_height);
                RenderFrame();
            }
            break;
        }
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

// DirectX 초기화 함수
bool InitDirectX(HWND hWnd) {
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 1;
    sd.BufferDesc.Width = g_width;
    sd.BufferDesc.Height = g_height;
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
        nullptr, 
        D3D_DRIVER_TYPE_HARDWARE, 
        nullptr, 
        createDeviceFlags,
        nullptr, 
        0, 
        D3D11_SDK_VERSION, 
        &sd, 
        &g_swapChain, 
        &g_device, 
        nullptr, 
        &g_context
    );
    if (FAILED(hr)) return false;

    // 2. 렌더 타겟 뷰
    ID3D11Texture2D* pBackBuffer = nullptr;
    g_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
    g_device->CreateRenderTargetView(pBackBuffer, nullptr, &g_renderTargetView);//중간파라미터는 mipmap같은 뷰설정
    pBackBuffer->Release();//mip map= 같은 텍스처를 여러해상도로 만들어놓은것

    return true;
}

void RenderFrame() {
    // 방어 코드: 장치나 RTV가 없으면 그리지 않음
    if (!g_context || !g_swapChain || !g_renderTargetView) return;

    // 1. 화면 지우기
    float clearColor[4] = { 0.1f, 0.2f, 0.4f, 1.0f }; 
    g_context->ClearRenderTargetView(g_renderTargetView, clearColor);

    // 2. 렌더 타겟 및 뷰포트 설정
    g_context->OMSetRenderTargets(1, &g_renderTargetView, nullptr);
    
    D3D11_VIEWPORT vp = {};
    vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;
    vp.Width = g_width;
    vp.Height = g_height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    g_context->RSSetViewports(1, &vp);

    // 3. 파이프라인 실행 (삼각형 그리기)
    if (g_appContext) {
        g_appContext->Render(g_context);
    }

    // 4. 화면 스왑
    g_swapChain->Present(1, 0);
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
    HWND hWnd = CreateWindowEx(0, className, WINDOW_NAME, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, g_width, g_height, NULL, NULL, hInstance, NULL);
    ShowWindow(hWnd, SW_SHOW);

    // DirectX 초기화
    if (!InitDirectX(hWnd)) {
        SPDLOG_ERROR("DirectX Init Failed");
        return -1;
    }

    // Context (파이프라인 및 리소스) 초기화
    g_appContext = Context::Create(g_device, g_context);
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
        } 
        
        else {
            // start render loop
            RenderFrame();
        }
    }

    return 0;
}

