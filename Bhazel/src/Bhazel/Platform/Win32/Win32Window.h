#pragma once

#include "Bhazel/Window.h"
#include "Bhazel/KeyCodes.h"


namespace BZ {


    class Win32Window : public Window {
    public:
        explicit Win32Window(const WindowData& data);
        ~Win32Window() override;

        void setVSync(bool enabled) override;
        void onUpdate() override;

        void* getNativeWindow() const override { return hWnd; }

    private:
        void init(const WindowData& data);
        void shutdown();

        LPCWSTR CLASS_NAME = L"BhazelWindowClass";
        HWND hWnd;
    };
}