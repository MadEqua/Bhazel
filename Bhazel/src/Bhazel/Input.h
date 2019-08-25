#pragma once

#include <utility>


namespace BZ {

    class Input {
    public:
        inline static bool isKeyPressed(int keycode) { return instance->isKeyPressedImpl(keycode); }
        
        inline static bool isMouseButtonPressed(int button) { return instance->isMouseButtonPressedImpl(button); }
        inline static std::pair<int, int> getMousePosition() { return instance->getMousePositionImpl(); }
        inline static int getMouseX() { return instance->getMouseXImpl(); }
        inline static int getMouseY() { return instance->getMouseYImpl(); }

    protected:
        virtual bool isKeyPressedImpl(int keycode) = 0;
        virtual bool isMouseButtonPressedImpl(int button) = 0;
        virtual std::pair<int, int> getMousePositionImpl() = 0;
        virtual int getMouseXImpl() = 0;
        virtual int getMouseYImpl() = 0;

    private:
        static Input *instance;
        static void init(void *nativeWindowHandle);

        friend class Application;
    };
}
