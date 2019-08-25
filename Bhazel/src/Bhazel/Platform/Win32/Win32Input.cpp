#include "bzpch.h"

#include "Win32Input.h"
#include "Bhazel/Application.h"
#include "Bhazel/KeyCodes.h"


namespace BZ {

    extern bool mouseButtons[BZ_MOUSE_BUTTON_LAST + 1];
    extern bool keys[BZ_KEY_LAST + 1];

    bool Win32Input::isKeyPressedImpl(int keycode) {
        BZ_CORE_ASSERT(keycode >= 0 && keycode <= BZ_KEY_LAST, "Invalid keycode!");
        return keys[keycode];
    }

    bool Win32Input::isMouseButtonPressedImpl(int button) {
        BZ_CORE_ASSERT(button >= 0 && button <= BZ_MOUSE_BUTTON_LAST, "Invalid button!");
        return mouseButtons[button];
    }

    std::pair<float, float> Win32Input::getMousePositionImpl() {
        return std::make_pair<float, float>(0, 0);
    }

    float Win32Input::getMouseXImpl() {
        return 0;
    }

    float Win32Input::getMouseYImpl() {
        return 0;
    }
}