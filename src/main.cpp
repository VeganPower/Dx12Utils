
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <windows.h>
// #include <dx12.h>

using Microsoft::WRL::ComPtr;

class WinApp
{
public:
    void show(int n_cmd_show)
    {
        ShowWindow(hwnd, n_cmd_show);
    }
    void run();
    static WinApp Build(HINSTANCE hInstance, LPCSTR title);
private:
    WinApp(HWND h) :hwnd{h} {}

    HWND hwnd;
};

// Main message handler for the sample.
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // DXSample* pSample = reinterpret_cast<DXSample*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (message)
    {
    case WM_CREATE:
        {
            // Save the DXSample* passed in to CreateWindow.
            LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
        }
        return 0;

    case WM_KEYDOWN:
        // if (pSample)
        // {
        //     pSample->OnKeyDown(static_cast<UINT8>(wParam));
        // }
        return 0;

    case WM_KEYUP:
        // if (pSample)
        // {
        //     pSample->OnKeyUp(static_cast<UINT8>(wParam));
        // }
        return 0;

    case WM_PAINT:
        // if (pSample)
        // {
            // pSample->OnUpdate();
            // pSample->OnRender();
        // }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

WinApp WinApp::Build(HINSTANCE hInstance, LPCSTR title)
{
    LPCSTR class_name = "Dx12Win32App";
    // Initialize the window class.
    WNDCLASSEX window_class = { 0 };
    window_class.cbSize = sizeof(WNDCLASSEX);
    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = WindowProc;
    window_class.hInstance = hInstance;
    window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    window_class.lpszClassName = class_name;
    RegisterClassEx(&window_class);


    RECT window_rect = { 0, 0, static_cast<LONG>(800), static_cast<LONG>(600) };
    AdjustWindowRect(&window_rect, WS_OVERLAPPEDWINDOW, FALSE);

    // Create the window and store a handle to it.
    auto wnd_h = CreateWindow(class_name, title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        window_rect.right - window_rect.left,
        window_rect.bottom - window_rect.top,
        nullptr,        // We have no parent window.
        nullptr,        // We aren't using menus.
        hInstance,
        nullptr);//
    return WinApp{wnd_h};
}

void WinApp::run()
{

    // Main sample loop.
    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        // Process any messages in the queue.
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // pSample->OnDestroy();

    // Return this part of the WM_QUIT message to Windows.
    //return static_cast<char>(msg.wParam);
}

class Dx12Base
{

    static const UINT FrameCount = 2;

    // Pipeline objects.
    ComPtr<IDXGISwapChain3>           swap_chain;
    ComPtr<ID3D12Device>              device;
    ComPtr<ID3D12Resource>            render_targets[FrameCount];
    ComPtr<ID3D12CommandAllocator>    command_allocator;
    ComPtr<ID3D12CommandQueue>        command_queue;
    ComPtr<ID3D12DescriptorHeap>      rtv_heap;
    ComPtr<ID3D12PipelineState>       pipeline_state;
    ComPtr<ID3D12GraphicsCommandList> command_list;

    UINT m_frameIndex;
    HANDLE m_fenceEvent;
    ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValue;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR argv, int cmd_show)
{
    auto window = WinApp::Build(hInstance, "Test");

    window.show(cmd_show);
    window.run();
    // Parse the command line parameters
    // int argc;
    // LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    // pSample->ParseCommandLineArgs(argv, argc);
    // LocalFree(argv);

    return 0;
}