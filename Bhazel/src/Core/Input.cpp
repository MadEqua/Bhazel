#include "bzpch.h"

#include "Input.h"


namespace BZ {

    void Input::init(void *nativeWindowHandle) {
        BZ_ASSERT_CORE(!Input::nativeWindowHandle, "NativeWindowHandle was already initialized!");
        BZ_ASSERT_CORE(nativeWindowHandle, "NativeWindowHandle in nullptr!");

        Input::nativeWindowHandle = nativeWindowHandle;
    }
}