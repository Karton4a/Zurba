#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>
#include <exception>
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include "Application.h"
#include <chrono>
#include <sstream>

static bool isRunning = true;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{

    // Register the window class.

    const wchar_t CLASS_NAME[] = L"Sample Window Class";

    WNDCLASS wc = { };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);
    // Create the window.
    unsigned width = 1920;
    unsigned height = 1080;
    Application* context = new Application{ width, height};
    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        L"Learn to Program Windows",    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        context        // Additional application data
    );

    if (hwnd == NULL)
    {
        return 0;
    }

    context->Init(hwnd);


    ShowWindow(hwnd, nCmdShow);
    
    // Run the message loop.
    float dt = 0;
    MSG msg = { };
    while (isRunning)
    {
        auto begin = std::chrono::high_resolution_clock::now();
        while (PeekMessage(&msg, NULL, 0, 0,PM_REMOVE) )
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        context->Update(dt);
        context->Render();
        auto end = std::chrono::high_resolution_clock::now();
        dt = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() / 1000.0f;
    }
    
    delete context;
    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    Application* ctx = reinterpret_cast<Application*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    switch (uMsg)
    {
    case WM_CREATE:
    {
        LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
    }
    return 0;
    case WM_DESTROY:

        PostQuitMessage(0);
        isRunning = false;
        return 0;

    case WM_KEYDOWN:
        if (ctx)
        {
            ctx->OnKeyDown(static_cast<UINT8>(wParam));
        }
        return 0;

    case WM_KEYUP:
        if (ctx)
        {
            ctx->OnKeyUp(static_cast<UINT8>(wParam));
        }
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}