#pragma once


namespace BZ {

    class Input {
    public:
        virtual bool isKeyPressed(int keycode) = 0;
        virtual bool isMouseButtonPressed(int button) = 0;
        virtual glm::ivec2 getMousePosition() = 0;
        virtual int getMouseX() = 0;
        virtual int getMouseY() = 0;

        static Input* create(void* nativeWindowHandle);
    };
}
