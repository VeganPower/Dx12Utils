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
    virtual void OnDestroy() = 0;
};


class Window
{
public:
    void show(int n_cmd_show)
    {
        ShowWindow(hwnd, n_cmd_show);
    }
    void run();

    HWND handler() const { return hwnd; }
    Resolution size() const { return resolution; }
private:
    friend WindowBuilder;

    HWND hwnd;
    Resolution resolution;
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
    operator Window(); // cast operator

    HINSTANCE hInstance;
    LPCSTR title = nullptr;
    AppListener* listener = nullptr;
    Resolution resolution;

};