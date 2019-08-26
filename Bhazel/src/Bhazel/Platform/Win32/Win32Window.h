#pragma once

#include "Bhazel/Window.h"
#include "Bhazel/KeyCodes.h"


namespace BZ {

    class Win32Window : public Window {
    public:
        explicit Win32Window(const WindowData& data);
        ~Win32Window() override;

        void onUpdate() override;

        void* getNativeWindowHandle() const override { return hWnd; }

        void setExtraHandlerFunction(LRESULT (CALLBACK *func)(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam));

    private:
        void init(const WindowData& data);
        void shutdown();

        LPCWSTR CLASS_NAME = L"BhazelWindowClass";
        HWND hWnd;
    };
}