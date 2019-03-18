#pragma once

#include "Bhazel/Input.h"

namespace BZ {

    class WindowsInput : public Input {
    protected:
        bool isKeyPressedImpl(int keycode) override;

        bool isMouseButtonPressedImpl(int button) override;
        std::pair<float, float> getMousePositionImpl() override;
        float getMouseXImpl() override;
        float getMouseYImpl() override;
    };
}