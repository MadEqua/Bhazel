#pragma once

#include "Bhazel/Input.h"

namespace BZ {

    class GlfwInput : public Input {
    protected:
        bool isKeyPressedImpl(int keycode) override;

        bool isMouseButtonPressedImpl(int button) override;
        std::pair<int, int> getMousePositionImpl() override;
        int getMouseXImpl() override;
        int getMouseYImpl() override;
    };
}