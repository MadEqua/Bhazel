#include "bzpch.h"

#include "Win32Window.h"
#include "Bhazel/Events/ApplicationEvent.h"
#include "Bhazel/Events/MouseEvent.h"
#include "Bhazel/Events/KeyEvent.h"
#include "Bhazel/KeyCodes.h"


namespace BZ {

    static short int keyCodes[512];

    static void createKeyTables() {
        memset(keyCodes, -1, sizeof(keyCodes));

        keyCodes[0x00B] = BZ_KEY_0;
        keyCodes[0x002] = BZ_KEY_1;
        keyCodes[0x003] = BZ_KEY_2;
        keyCodes[0x004] = BZ_KEY_3;
        keyCodes[0x005] = BZ_KEY_4;
        keyCodes[0x006] = BZ_KEY_5;
        keyCodes[0x007] = BZ_KEY_6;
        keyCodes[0x008] = BZ_KEY_7;
        keyCodes[0x009] = BZ_KEY_8;
        keyCodes[0x00A] = BZ_KEY_9;
        keyCodes[0x01E] = BZ_KEY_A;
        keyCodes[0x030] = BZ_KEY_B;
        keyCodes[0x02E] = BZ_KEY_C;
        keyCodes[0x020] = BZ_KEY_D;
        keyCodes[0x012] = BZ_KEY_E;
        keyCodes[0x021] = BZ_KEY_F;
        keyCodes[0x022] = BZ_KEY_G;
        keyCodes[0x023] = BZ_KEY_H;
        keyCodes[0x017] = BZ_KEY_I;
        keyCodes[0x024] = BZ_KEY_J;
        keyCodes[0x025] = BZ_KEY_K;
        keyCodes[0x026] = BZ_KEY_L;
        keyCodes[0x032] = BZ_KEY_M;
        keyCodes[0x031] = BZ_KEY_N;
        keyCodes[0x018] = BZ_KEY_O;
        keyCodes[0x019] = BZ_KEY_P;
        keyCodes[0x010] = BZ_KEY_Q;
        keyCodes[0x013] = BZ_KEY_R;
        keyCodes[0x01F] = BZ_KEY_S;
        keyCodes[0x014] = BZ_KEY_T;
        keyCodes[0x016] = BZ_KEY_U;
        keyCodes[0x02F] = BZ_KEY_V;
        keyCodes[0x011] = BZ_KEY_W;
        keyCodes[0x02D] = BZ_KEY_X;
        keyCodes[0x015] = BZ_KEY_Y;
        keyCodes[0x02C] = BZ_KEY_Z;

        keyCodes[0x028] = BZ_KEY_APOSTROPHE;
        keyCodes[0x02B] = BZ_KEY_BACKSLASH;
        keyCodes[0x033] = BZ_KEY_COMMA;
        keyCodes[0x00D] = BZ_KEY_EQUAL;
        keyCodes[0x029] = BZ_KEY_GRAVE_ACCENT;
        keyCodes[0x01A] = BZ_KEY_LEFT_BRACKET;
        keyCodes[0x00C] = BZ_KEY_MINUS;
        keyCodes[0x034] = BZ_KEY_PERIOD;
        keyCodes[0x01B] = BZ_KEY_RIGHT_BRACKET;
        keyCodes[0x027] = BZ_KEY_SEMICOLON;
        keyCodes[0x035] = BZ_KEY_SLASH;
        keyCodes[0x056] = BZ_KEY_WORLD_2;

        keyCodes[0x00E] = BZ_KEY_BACKSPACE;
        keyCodes[0x153] = BZ_KEY_DELETE;
        keyCodes[0x14F] = BZ_KEY_END;
        keyCodes[0x01C] = BZ_KEY_ENTER;
        keyCodes[0x001] = BZ_KEY_ESCAPE;
        keyCodes[0x147] = BZ_KEY_HOME;
        keyCodes[0x152] = BZ_KEY_INSERT;
        keyCodes[0x15D] = BZ_KEY_MENU;
        keyCodes[0x151] = BZ_KEY_PAGE_DOWN;
        keyCodes[0x149] = BZ_KEY_PAGE_UP;
        keyCodes[0x045] = BZ_KEY_PAUSE;
        keyCodes[0x146] = BZ_KEY_PAUSE;
        keyCodes[0x039] = BZ_KEY_SPACE;
        keyCodes[0x00F] = BZ_KEY_TAB;
        keyCodes[0x03A] = BZ_KEY_CAPS_LOCK;
        keyCodes[0x145] = BZ_KEY_NUM_LOCK;
        keyCodes[0x046] = BZ_KEY_SCROLL_LOCK;
        keyCodes[0x03B] = BZ_KEY_F1;
        keyCodes[0x03C] = BZ_KEY_F2;
        keyCodes[0x03D] = BZ_KEY_F3;
        keyCodes[0x03E] = BZ_KEY_F4;
        keyCodes[0x03F] = BZ_KEY_F5;
        keyCodes[0x040] = BZ_KEY_F6;
        keyCodes[0x041] = BZ_KEY_F7;
        keyCodes[0x042] = BZ_KEY_F8;
        keyCodes[0x043] = BZ_KEY_F9;
        keyCodes[0x044] = BZ_KEY_F10;
        keyCodes[0x057] = BZ_KEY_F11;
        keyCodes[0x058] = BZ_KEY_F12;
        keyCodes[0x064] = BZ_KEY_F13;
        keyCodes[0x065] = BZ_KEY_F14;
        keyCodes[0x066] = BZ_KEY_F15;
        keyCodes[0x067] = BZ_KEY_F16;
        keyCodes[0x068] = BZ_KEY_F17;
        keyCodes[0x069] = BZ_KEY_F18;
        keyCodes[0x06A] = BZ_KEY_F19;
        keyCodes[0x06B] = BZ_KEY_F20;
        keyCodes[0x06C] = BZ_KEY_F21;
        keyCodes[0x06D] = BZ_KEY_F22;
        keyCodes[0x06E] = BZ_KEY_F23;
        keyCodes[0x076] = BZ_KEY_F24;
        keyCodes[0x038] = BZ_KEY_LEFT_ALT;
        keyCodes[0x01D] = BZ_KEY_LEFT_CONTROL;
        keyCodes[0x02A] = BZ_KEY_LEFT_SHIFT;
        keyCodes[0x15B] = BZ_KEY_LEFT_SUPER;
        keyCodes[0x137] = BZ_KEY_PRINT_SCREEN;
        keyCodes[0x138] = BZ_KEY_RIGHT_ALT;
        keyCodes[0x11D] = BZ_KEY_RIGHT_CONTROL;
        keyCodes[0x036] = BZ_KEY_RIGHT_SHIFT;
        keyCodes[0x15C] = BZ_KEY_RIGHT_SUPER;
        keyCodes[0x150] = BZ_KEY_DOWN;
        keyCodes[0x14B] = BZ_KEY_LEFT;
        keyCodes[0x14D] = BZ_KEY_RIGHT;
        keyCodes[0x148] = BZ_KEY_UP;

        keyCodes[0x052] = BZ_KEY_KP_0;
        keyCodes[0x04F] = BZ_KEY_KP_1;
        keyCodes[0x050] = BZ_KEY_KP_2;
        keyCodes[0x051] = BZ_KEY_KP_3;
        keyCodes[0x04B] = BZ_KEY_KP_4;
        keyCodes[0x04C] = BZ_KEY_KP_5;
        keyCodes[0x04D] = BZ_KEY_KP_6;
        keyCodes[0x047] = BZ_KEY_KP_7;
        keyCodes[0x048] = BZ_KEY_KP_8;
        keyCodes[0x049] = BZ_KEY_KP_9;
        keyCodes[0x04E] = BZ_KEY_KP_ADD;
        keyCodes[0x053] = BZ_KEY_KP_DECIMAL;
        keyCodes[0x135] = BZ_KEY_KP_DIVIDE;
        keyCodes[0x11C] = BZ_KEY_KP_ENTER;
        keyCodes[0x059] = BZ_KEY_KP_EQUAL;
        keyCodes[0x037] = BZ_KEY_KP_MULTIPLY;
        keyCodes[0x04A] = BZ_KEY_KP_SUBTRACT;
    }

    static int translateKey(WPARAM wParam, LPARAM lParam) {
        // The Ctrl keys require special handling
        if(wParam == VK_CONTROL) {
            MSG next;
            DWORD time;

            // Right side keys have the extended key bit set
            if(lParam & 0x01000000)
                return BZ_KEY_RIGHT_CONTROL;

            // HACK: Alt Gr sends Left Ctrl and then Right Alt in close sequence
            //       We only want the Right Alt message, so if the next message is
            //       Right Alt we ignore this (synthetic) Left Ctrl message
            time = GetMessageTime();

            if(PeekMessageW(&next, NULL, 0, 0, PM_NOREMOVE)) {
                if(next.message == WM_KEYDOWN ||
                   next.message == WM_SYSKEYDOWN ||
                   next.message == WM_KEYUP ||
                   next.message == WM_SYSKEYUP) {
                    if(next.wParam == VK_MENU &&
                        (next.lParam & 0x01000000) && next.time == time) {
                        // Next message is Right Alt down so discard this
                        return -1;
                    }
                }
            }
            return BZ_KEY_LEFT_CONTROL;
        }

        if(wParam == VK_PROCESSKEY) {
            // IME notifies that keys have been filtered by setting the virtual
            // key-code to VK_PROCESSKEY
            return -1;
        }

        return keyCodes[HIWORD(lParam) & 0x1FF];
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
            case WM_SYSKEYDOWN:
            {
                int translatedKey = translateKey(wParam, lParam);
                if(translatedKey >= 0) {
                    WindowData *windowData = (WindowData*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
                    KeyPressedEvent event(translatedKey, LOWORD(lParam));
                    windowData->eventCallback(event);
                }
                break;
            }
            case WM_KEYUP:
            case WM_SYSKEYUP:
            {
                int translatedKey = translateKey(wParam, lParam);
                if(translatedKey >= 0) {
                    WindowData *windowData = (WindowData*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
                    KeyReleasedEvent event(translatedKey);
                    windowData->eventCallback(event);
                }
                break;
            }
            case WM_CHAR:
            {
                if(wParam > 32 && (wParam < 126 || wParam > 160)) {
                    WindowData *windowData = (WindowData*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
                    KeyTypedEvent event(wParam);
                    windowData->eventCallback(event);
                }
                break;
            }
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
            {
                WindowData *windowData = (WindowData*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
                MouseButtonPressedEvent event(BZ_MOUSE_BUTTON_LEFT);
                windowData->eventCallback(event);
                break;
            }
            case WM_LBUTTONUP:
            {
                WindowData *windowData = (WindowData*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
                MouseButtonReleasedEvent event(BZ_MOUSE_BUTTON_LEFT);
                windowData->eventCallback(event);
                break;
            }
            case WM_MBUTTONDOWN:
            {
                WindowData *windowData = (WindowData*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
                MouseButtonPressedEvent event(BZ_MOUSE_BUTTON_MIDDLE);
                windowData->eventCallback(event);
                break;
            }
            case WM_MBUTTONUP:
            {
                WindowData *windowData = (WindowData*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
                MouseButtonReleasedEvent event(BZ_MOUSE_BUTTON_MIDDLE);
                windowData->eventCallback(event);
                break;
            }
            case WM_RBUTTONDOWN:
            {
                WindowData *windowData = (WindowData*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
                MouseButtonPressedEvent event(BZ_MOUSE_BUTTON_RIGHT);
                windowData->eventCallback(event);
                break;
            }
            case WM_RBUTTONUP:
            {
                WindowData *windowData = (WindowData*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
                MouseButtonReleasedEvent event(BZ_MOUSE_BUTTON_RIGHT);
                windowData->eventCallback(event);
                break;
            }
            case WM_XBUTTONDOWN:
            {
                WindowData *windowData = (WindowData*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
                MouseButtonPressedEvent event(GET_XBUTTON_WPARAM(wParam) + BZ_MOUSE_BUTTON_3);
                windowData->eventCallback(event);
                break;
            }
            case WM_XBUTTONUP:
            {
                WindowData *windowData = (WindowData*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
                MouseButtonReleasedEvent event(GET_XBUTTON_WPARAM(wParam) + BZ_MOUSE_BUTTON_3);
                windowData->eventCallback(event);
                break;
            }
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
        createKeyTables();
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