#include "bzpch.h"

#include "Win32Input.h"
#include "Bhazel/Application.h"


namespace BZ {

    bool Win32Input::isKeyPressedImpl(int keycode) {
        return false;
    }

    bool Win32Input::isMouseButtonPressedImpl(int button) {
        return false;
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