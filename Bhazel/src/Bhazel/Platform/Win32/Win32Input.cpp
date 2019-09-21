#include "bzpch.h"

#include <Windows.h>

#include <bitset>

#include "Win32Input.h"
#include "Bhazel/Application.h"
#include "Bhazel/KeyCodes.h"


namespace BZ {

    extern std::bitset<BZ_MOUSE_BUTTON_LAST + 1> mouseButtons;
    extern std::bitset<BZ_KEY_LAST + 1> keys;

    bool Win32Input::isKeyPressedImpl(int keycode) {
        if(keycode < 0 || keycode > BZ_KEY_LAST) {
            BZ_LOG_CORE_ERROR("Win32Input. Invalid keycode!");
            return false;
        }
        return keys[keycode];
    }

    bool Win32Input::isMouseButtonPressedImpl(int button) {
        if(button < 0 || button > BZ_MOUSE_BUTTON_LAST) {
            BZ_LOG_CORE_ERROR("Win32Input. Invalid mouse button!");
            return false;
        }
        return mouseButtons[button];
    }

    glm::ivec2 Win32Input::getMousePositionImpl() {
        POINT pos = {};
        if(GetCursorPos(&pos)) {
            ScreenToClient(nativeWindow, &pos);
        }
        return glm::ivec2(static_cast<int>(pos.x), static_cast<int>(pos.y));
    }

    int Win32Input::getMouseXImpl() {
        return getMousePositionImpl().x;
    }

    int Win32Input::getMouseYImpl() {
        return getMousePositionImpl().y;
    }
}