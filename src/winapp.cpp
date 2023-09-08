#include "common.h"
#include "winapp.h"

// Main message handler for the sample.
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    AppListener* listener = reinterpret_cast<AppListener*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (message)
    {
    case WM_CREATE:
        {
            // Save the AppListener* passed in to CreateWindow.
            LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
        }
        return 0;

    case WM_KEYDOWN:
        if (listener)
        {
            listener->OnKeyDown(static_cast<char>(wParam));
        }
        return 0;

    case WM_KEYUP:
        if (wParam == VK_ESCAPE)
            PostQuitMessage(0);
        if (listener)
        {
            listener->OnKeyUp(static_cast<char>(wParam));
        }
        return 0;

    case WM_PAINT:
        if (listener)
        {
            listener->OnUpdate();
            listener->OnRender();
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

WindowBuilder::operator WinApp()
{
    LPCSTR class_name = "Dx12Win32App";
    // Initialize the window class.
    WNDCLASSEX window_class = { 0 };
    window_class.cbSize        = sizeof(WNDCLASSEX);
    window_class.style         = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc   = WindowProc;
    window_class.hInstance     = hInstance;
    window_class.hCursor       = LoadCursor(NULL, IDC_ARROW);
    window_class.lpszClassName = class_name;
    RegisterClassEx(&window_class);

    RECT window_rect = { 0, 0, static_cast<LONG>(resolution.width), static_cast<LONG>(resolution.height) };
    AdjustWindowRect(&window_rect, WS_OVERLAPPEDWINDOW, FALSE);

    // Create the window and store a handle to it.
    auto wnd_h = CreateWindow(class_name, title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        resolution.width, resolution.height,
        nullptr,        // We have no parent window.
        nullptr,        // We aren't using menus.
        hInstance, listener);
    return WinApp{wnd_h};
}

void WinApp::run()
{
    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
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