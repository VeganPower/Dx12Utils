#pragma once
#include "resolution.h"

class WindowBuilder;
class AppListener
{
public:
    virtual ~AppListener() {};

    virtual void OnKeyDown(char ch) = 0;
    virtual void OnKeyUp(char ch) = 0;
    virtual void OnUpdate() = 0;
    virtual void OnRender() = 0;
};


class WinApp
{
public:
    void show(int n_cmd_show)
    {
        ShowWindow(hwnd, n_cmd_show);
    }
    void run();
private:
    friend WindowBuilder;

    WinApp(HWND h) :hwnd{h} {}
    HWND hwnd;
};

struct WindowBuilder
{
    WindowBuilder(HINSTANCE h)
        : hInstance { h }
        , resolution { 800, 600 }
    {
    }
    WindowBuilder& with_title(const char* t) { title = t; return *this; }
    WindowBuilder& with_size(Resolution res) { resolution = res; return *this; }
    WindowBuilder& with_listener(AppListener* l) { listener = l; return *this; }
    operator WinApp(); // cast operator

    HINSTANCE hInstance;
    LPCSTR title = nullptr;
    AppListener* listener = nullptr;
    Resolution resolution;

};