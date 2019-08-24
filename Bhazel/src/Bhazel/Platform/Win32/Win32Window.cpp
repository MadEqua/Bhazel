#include "bzpch.h"

#include "Win32Window.h"
#include "Bhazel/Events/ApplicationEvent.h"
#include "Bhazel/Events/MouseEvent.h"
#include "Bhazel/Events/KeyEvent.h"
#include "Bhazel/KeyCodes.h"


namespace BZ {

    static void processKeyDown(uint32 virtualKeyCode, uint32 repeatCount, WindowData &windowData) {
        KeyPressedEvent event(virtualKeyCode, repeatCount); //TODO convert the key codes
        windowData.eventCallback(event);
    }
    
    static void processKeyUp(uint32 virtualKeyCode, WindowData &windowData) {
        KeyReleasedEvent event(virtualKeyCode); //TODO convert the key codes
        windowData.eventCallback(event);
    }

    static void processKeyChar(uint32 charCode, WindowData &windowData) {
        KeyTypedEvent event(charCode); //TODO convert the key codes
        windowData.eventCallback(event);
    }

    static void processMouseButtonUp(uint32 buttonCode, WindowData &windowData) {
        MouseButtonPressedEvent event(buttonCode);
        windowData.eventCallback(event);
    }

    static void processMouseButtonDown(uint32 buttonCode, WindowData &windowData) {
        MouseButtonReleasedEvent event(buttonCode);
        windowData.eventCallback(event);
    }

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {

        switch(msg) {
        case WM_NCCREATE:
            //Deal with the custom data
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR) ((CREATESTRUCT*) lParam)->lpCreateParams);
            break;

        case WM_SIZE:
        {
            WindowData *windowData = (WindowData*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
            WindowResizeEvent event(LOWORD(lParam), HIWORD(lParam));
            windowData->eventCallback(event);
            break;
        }
        case WM_CLOSE:
        {
            WindowData *windowData = (WindowData*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
            WindowCloseEvent event;
            windowData->eventCallback(event);
            PostQuitMessage(0); //Break the event consumption ASAP
            break;
        }
        case WM_KEYDOWN:
            processKeyDown(wParam, LOWORD(lParam), *((WindowData*) GetWindowLongPtr(hWnd, GWLP_USERDATA)));
            break;
        case WM_KEYUP:
            processKeyUp(wParam, *((WindowData*) GetWindowLongPtr(hWnd, GWLP_USERDATA)));
            break;
        case WM_CHAR:
            processKeyChar(wParam, *((WindowData*) GetWindowLongPtr(hWnd, GWLP_USERDATA)));
            break;
        case WM_MOUSEMOVE:
        {
            WindowData *windowData = (WindowData*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
            MouseMovedEvent event(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            windowData->eventCallback(event);
            break;
        }
        case WM_MOUSEWHEEL:
        {
            WindowData *windowData = (WindowData*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
            MouseScrolledEvent event(0, GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
            windowData->eventCallback(event);
            break;
        }
        case WM_LBUTTONDOWN:
            processMouseButtonDown(BZ_MOUSE_BUTTON_LEFT, *((WindowData*) GetWindowLongPtr(hWnd, GWLP_USERDATA)));
            break;
        case WM_LBUTTONUP:
            processMouseButtonUp(BZ_MOUSE_BUTTON_LEFT, *((WindowData*) GetWindowLongPtr(hWnd, GWLP_USERDATA)));
            break;
        case WM_MBUTTONDOWN:
            processMouseButtonDown(BZ_MOUSE_BUTTON_MIDDLE, *((WindowData*) GetWindowLongPtr(hWnd, GWLP_USERDATA)));
            break;
        case WM_MBUTTONUP:
            processMouseButtonUp(BZ_MOUSE_BUTTON_MIDDLE, *((WindowData*) GetWindowLongPtr(hWnd, GWLP_USERDATA)));
            break;
        case WM_RBUTTONDOWN:
            processMouseButtonDown(BZ_MOUSE_BUTTON_RIGHT, *((WindowData*) GetWindowLongPtr(hWnd, GWLP_USERDATA)));
            break;
        case WM_RBUTTONUP:
            processMouseButtonUp(BZ_MOUSE_BUTTON_RIGHT, *((WindowData*) GetWindowLongPtr(hWnd, GWLP_USERDATA)));
            break;
        case WM_XBUTTONDOWN:
            processMouseButtonDown(GET_XBUTTON_WPARAM(wParam) + BZ_MOUSE_BUTTON_3, *((WindowData*) GetWindowLongPtr(hWnd, GWLP_USERDATA)));
            break;
        case WM_XBUTTONUP:
            processMouseButtonUp(GET_XBUTTON_WPARAM(wParam) + BZ_MOUSE_BUTTON_3, *((WindowData*) GetWindowLongPtr(hWnd, GWLP_USERDATA)));
            break;
        }
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }


    Win32Window::Win32Window(const WindowData &data) {
        init(data);
    }

    Win32Window::~Win32Window() {
        shutdown();
    }

    void Win32Window::init(const WindowData &data) {
        windowData = data;

        BZ_LOG_CORE_INFO("Creating Win32 window {0} ({1}, {2})", data.title, data.width, data.height);

        //register window class
        WNDCLASSEXW wndClass = {0};
        wndClass.cbSize = sizeof(wndClass);
        wndClass.style = CS_OWNDC;
        wndClass.lpfnWndProc = WndProc;
        wndClass.cbClsExtra = 0;
        wndClass.cbWndExtra = 0;
        wndClass.hInstance = GetModuleHandle(nullptr);
        wndClass.hIcon = nullptr;
        wndClass.hCursor = nullptr;
        wndClass.hbrBackground = nullptr;
        wndClass.lpszMenuName = nullptr;
        wndClass.lpszClassName = CLASS_NAME;
        wndClass.hIconSm = nullptr;
        RegisterClassEx(&wndClass);

        //create window instance
        hWnd = CreateWindowEx(
            0,
            CLASS_NAME,
            std::wstring(data.title.begin(), data.title.end()).c_str(),
            WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CAPTION | WS_SYSMENU | WS_SIZEBOX,
            CW_USEDEFAULT, CW_USEDEFAULT,
            data.width, data.height,
            nullptr,
            nullptr,
            GetModuleHandle(nullptr),
            (LPVOID)&windowData //Send custom data to the WndProc callback
        );

        BZ_CORE_ASSERT(hWnd, "Couldn't create window!");
        ShowWindow(hWnd, SW_SHOW);
    }

    void Win32Window::shutdown() {
        UnregisterClass(CLASS_NAME, GetModuleHandle(nullptr));
        DestroyWindow(hWnd);
    }

    void Win32Window::onUpdate() {
        MSG msg;
        while(GetMessage(&msg, nullptr, 0, 0) > 0) { //TODO this never breaks unless window close
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            //BZ_LOG_CORE_DEBUG("received msg!");
        }
        BZ_LOG_CORE_DEBUG("out of the loop");
        //graphicsContext->swapBuffers();
    }

    void Win32Window::setVSync(bool enabled) {
        Window::setVSync(enabled);
        //TODO
    }
}