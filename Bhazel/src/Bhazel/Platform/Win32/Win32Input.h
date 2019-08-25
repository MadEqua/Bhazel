#pragma once

#include "Bhazel/Input.h"


namespace BZ {

    class Win32Input : public Input {
    public:
        explicit Win32Input(HWND nativeWindow) : nativeWindow(nativeWindow) {}

    protected:
        bool isKeyPressedImpl(int keycode) override;

        bool isMouseButtonPressedImpl(int button) override;
        std::pair<int, int> getMousePositionImpl() override;
        int getMouseXImpl() override;
        int getMouseYImpl() override;

    private:
        HWND nativeWindow;
    };
}