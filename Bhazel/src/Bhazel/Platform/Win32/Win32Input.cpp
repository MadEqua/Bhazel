#include "bzpch.h"

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

    std::pair<int, int> Win32Input::getMousePositionImpl() {
        POINT pos = {0};
        if(GetCursorPos(&pos)) {
            ScreenToClient(nativeWindow, &pos);
        }
        return std::make_pair<int, int>(static_cast<int>(pos.x), static_cast<int>(pos.y));
    }

    int Win32Input::getMouseXImpl() {
        auto[x, y] = getMousePositionImpl();
        return x;
    }

    int Win32Input::getMouseYImpl() {
        auto[x, y] = getMousePositionImpl();
        return y;
    }
}