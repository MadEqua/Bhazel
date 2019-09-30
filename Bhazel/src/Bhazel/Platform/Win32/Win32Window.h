#pragma once

#include "Bhazel/Window.h"


namespace BZ {

    class Win32Window : public Window {
    public:
        explicit Win32Window(const WindowData &data, EventCallbackFn eventCallback);
        virtual ~Win32Window() override;

        virtual void pollEvents() override;
        virtual void setTitle(const char* title) override;

        virtual void* getNativeWindowHandle() const override { return hWnd; }

        //Win32Window will call this Handler function after its own handling of the events
        void setExtraHandlerFunction(LRESULT (CALLBACK *func)(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam));

    private:
        void init();
        void shutdown();

        LPCWSTR CLASS_NAME = L"BhazelWindowClass";
        HWND hWnd;

        bool inited = false;

        friend LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    };
}