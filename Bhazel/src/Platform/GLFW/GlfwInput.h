#pragma once

#include "Core/Input.h"

namespace BZ {

    class GlfwInput : public Input {
    public:
        bool isKeyPressed(int keycode) override;

        bool isMouseButtonPressed(int button) override;
        glm::ivec2 getMousePosition() override;
        int getMouseX() override;
        int getMouseY() override;
    };
}