#pragma once

#include "Core/Input.h"


namespace BZ {

    class Win32Input : public Input {
    public:
        explicit Win32Input(void *nativeWindowHandle) : nativeWindow(static_cast<HWND>(nativeWindowHandle)) {}

        bool isKeyPressed(int keycode) override;

        bool isMouseButtonPressed(int button) override;
        glm::ivec2 getMousePosition() override;
        int getMouseX() override;
        int getMouseY() override;

    private:
        HWND nativeWindow;
    };
}