#pragma once

#include "Bhazel/Window.h"
#include "Bhazel/KeyCodes.h"


namespace BZ {

    class Win32Window : public Window {
    public:
        explicit Win32Window(EventCallbackFn eventCallback, const WindowData &data);
        virtual ~Win32Window() override;

        virtual void onUpdate() override;
        virtual void setTitle(const std::string &title) override;

        virtual void* getNativeWindowHandle() const override { return hWnd; }

        void setExtraHandlerFunction(LRESULT (CALLBACK *func)(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam));

    private:
        void init();
        void shutdown();

        LPCWSTR CLASS_NAME = L"BhazelWindowClass";
        HWND hWnd;
    };
}