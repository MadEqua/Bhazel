#include "bzpch.h"

#include "Window.h"


namespace BZ {

    Window::Window(const WindowData &data, EventCallbackFn eventCallback) :
        eventCallback(eventCallback), data(data) {
    }
}