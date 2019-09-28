#include "bzpch.h"

#include <Windows.h>
#include <Windowsx.h>

#include <bitset>

#include "Win32Window.h"
#include "Bhazel/Events/ApplicationEvent.h"
#include "Bhazel/Events/MouseEvent.h"
#include "Bhazel/Events/KeyEvent.h"

#include "Bhazel/Platform/D3D11/D3D11Context.h"


namespace BZ {

    //TODO: These globals will exist even when not using Win32Window
    std::bitset<BZ_MOUSE_BUTTON_LAST + 1> mouseButtons;
    std::bitset<BZ_KEY_LAST + 1> keys;
    static int16 keyTranslationTable[512];


    BOOL isWindows10BuildOrGreaterWin32(WORD build) {
        OSVERSIONINFOEXW osvi = {sizeof(osvi), 10, 0, build};
        DWORD mask = VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER;
        ULONGLONG cond = VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL);
        cond = VerSetConditionMask(cond, VER_MINORVERSION, VER_GREATER_EQUAL);
        cond = VerSetConditionMask(cond, VER_BUILDNUMBER, VER_GREATER_EQUAL);
        return VerifyVersionInfoW(&osvi, mask, cond) == 0;
    }

#define isWindows10AnniversaryUpdateOrGreaterWin32() isWindows10BuildOrGreaterWin32(14393)


    static void createKeyTranslationTable() {
        memset(keyTranslationTable, -1, sizeof(keyTranslationTable));

        keyTranslationTable[0x00B] = BZ_KEY_0;
        keyTranslationTable[0x002] = BZ_KEY_1;
        keyTranslationTable[0x003] = BZ_KEY_2;
        keyTranslationTable[0x004] = BZ_KEY_3;
        keyTranslationTable[0x005] = BZ_KEY_4;
        keyTranslationTable[0x006] = BZ_KEY_5;
        keyTranslationTable[0x007] = BZ_KEY_6;
        keyTranslationTable[0x008] = BZ_KEY_7;
        keyTranslationTable[0x009] = BZ_KEY_8;
        keyTranslationTable[0x00A] = BZ_KEY_9;
        keyTranslationTable[0x01E] = BZ_KEY_A;
        keyTranslationTable[0x030] = BZ_KEY_B;
        keyTranslationTable[0x02E] = BZ_KEY_C;
        keyTranslationTable[0x020] = BZ_KEY_D;
        keyTranslationTable[0x012] = BZ_KEY_E;
        keyTranslationTable[0x021] = BZ_KEY_F;
        keyTranslationTable[0x022] = BZ_KEY_G;
        keyTranslationTable[0x023] = BZ_KEY_H;
        keyTranslationTable[0x017] = BZ_KEY_I;
        keyTranslationTable[0x024] = BZ_KEY_J;
        keyTranslationTable[0x025] = BZ_KEY_K;
        keyTranslationTable[0x026] = BZ_KEY_L;
        keyTranslationTable[0x032] = BZ_KEY_M;
        keyTranslationTable[0x031] = BZ_KEY_N;
        keyTranslationTable[0x018] = BZ_KEY_O;
        keyTranslationTable[0x019] = BZ_KEY_P;
        keyTranslationTable[0x010] = BZ_KEY_Q;
        keyTranslationTable[0x013] = BZ_KEY_R;
        keyTranslationTable[0x01F] = BZ_KEY_S;
        keyTranslationTable[0x014] = BZ_KEY_T;
        keyTranslationTable[0x016] = BZ_KEY_U;
        keyTranslationTable[0x02F] = BZ_KEY_V;
        keyTranslationTable[0x011] = BZ_KEY_W;
        keyTranslationTable[0x02D] = BZ_KEY_X;
        keyTranslationTable[0x015] = BZ_KEY_Y;
        keyTranslationTable[0x02C] = BZ_KEY_Z;

        keyTranslationTable[0x028] = BZ_KEY_APOSTROPHE;
        keyTranslationTable[0x02B] = BZ_KEY_BACKSLASH;
        keyTranslationTable[0x033] = BZ_KEY_COMMA;
        keyTranslationTable[0x00D] = BZ_KEY_EQUAL;
        keyTranslationTable[0x029] = BZ_KEY_GRAVE_ACCENT;
        keyTranslationTable[0x01A] = BZ_KEY_LEFT_BRACKET;
        keyTranslationTable[0x00C] = BZ_KEY_MINUS;
        keyTranslationTable[0x034] = BZ_KEY_PERIOD;
        keyTranslationTable[0x01B] = BZ_KEY_RIGHT_BRACKET;
        keyTranslationTable[0x027] = BZ_KEY_SEMICOLON;
        keyTranslationTable[0x035] = BZ_KEY_SLASH;
        keyTranslationTable[0x056] = BZ_KEY_WORLD_2;

        keyTranslationTable[0x00E] = BZ_KEY_BACKSPACE;
        keyTranslationTable[0x153] = BZ_KEY_DELETE;
        keyTranslationTable[0x14F] = BZ_KEY_END;
        keyTranslationTable[0x01C] = BZ_KEY_ENTER;
        keyTranslationTable[0x001] = BZ_KEY_ESCAPE;
        keyTranslationTable[0x147] = BZ_KEY_HOME;
        keyTranslationTable[0x152] = BZ_KEY_INSERT;
        keyTranslationTable[0x15D] = BZ_KEY_MENU;
        keyTranslationTable[0x151] = BZ_KEY_PAGE_DOWN;
        keyTranslationTable[0x149] = BZ_KEY_PAGE_UP;
        keyTranslationTable[0x045] = BZ_KEY_PAUSE;
        keyTranslationTable[0x146] = BZ_KEY_PAUSE;
        keyTranslationTable[0x039] = BZ_KEY_SPACE;
        keyTranslationTable[0x00F] = BZ_KEY_TAB;
        keyTranslationTable[0x03A] = BZ_KEY_CAPS_LOCK;
        keyTranslationTable[0x145] = BZ_KEY_NUM_LOCK;
        keyTranslationTable[0x046] = BZ_KEY_SCROLL_LOCK;
        keyTranslationTable[0x03B] = BZ_KEY_F1;
        keyTranslationTable[0x03C] = BZ_KEY_F2;
        keyTranslationTable[0x03D] = BZ_KEY_F3;
        keyTranslationTable[0x03E] = BZ_KEY_F4;
        keyTranslationTable[0x03F] = BZ_KEY_F5;
        keyTranslationTable[0x040] = BZ_KEY_F6;
        keyTranslationTable[0x041] = BZ_KEY_F7;
        keyTranslationTable[0x042] = BZ_KEY_F8;
        keyTranslationTable[0x043] = BZ_KEY_F9;
        keyTranslationTable[0x044] = BZ_KEY_F10;
        keyTranslationTable[0x057] = BZ_KEY_F11;
        keyTranslationTable[0x058] = BZ_KEY_F12;
        keyTranslationTable[0x064] = BZ_KEY_F13;
        keyTranslationTable[0x065] = BZ_KEY_F14;
        keyTranslationTable[0x066] = BZ_KEY_F15;
        keyTranslationTable[0x067] = BZ_KEY_F16;
        keyTranslationTable[0x068] = BZ_KEY_F17;
        keyTranslationTable[0x069] = BZ_KEY_F18;
        keyTranslationTable[0x06A] = BZ_KEY_F19;
        keyTranslationTable[0x06B] = BZ_KEY_F20;
        keyTranslationTable[0x06C] = BZ_KEY_F21;
        keyTranslationTable[0x06D] = BZ_KEY_F22;
        keyTranslationTable[0x06E] = BZ_KEY_F23;
        keyTranslationTable[0x076] = BZ_KEY_F24;
        keyTranslationTable[0x038] = BZ_KEY_LEFT_ALT;
        keyTranslationTable[0x01D] = BZ_KEY_LEFT_CONTROL;
        keyTranslationTable[0x02A] = BZ_KEY_LEFT_SHIFT;
        keyTranslationTable[0x15B] = BZ_KEY_LEFT_SUPER;
        keyTranslationTable[0x137] = BZ_KEY_PRINT_SCREEN;
        keyTranslationTable[0x138] = BZ_KEY_RIGHT_ALT;
        keyTranslationTable[0x11D] = BZ_KEY_RIGHT_CONTROL;
        keyTranslationTable[0x036] = BZ_KEY_RIGHT_SHIFT;
        keyTranslationTable[0x15C] = BZ_KEY_RIGHT_SUPER;
        keyTranslationTable[0x150] = BZ_KEY_DOWN;
        keyTranslationTable[0x14B] = BZ_KEY_LEFT;
        keyTranslationTable[0x14D] = BZ_KEY_RIGHT;
        keyTranslationTable[0x148] = BZ_KEY_UP;

        keyTranslationTable[0x052] = BZ_KEY_KP_0;
        keyTranslationTable[0x04F] = BZ_KEY_KP_1;
        keyTranslationTable[0x050] = BZ_KEY_KP_2;
        keyTranslationTable[0x051] = BZ_KEY_KP_3;
        keyTranslationTable[0x04B] = BZ_KEY_KP_4;
        keyTranslationTable[0x04C] = BZ_KEY_KP_5;
        keyTranslationTable[0x04D] = BZ_KEY_KP_6;
        keyTranslationTable[0x047] = BZ_KEY_KP_7;
        keyTranslationTable[0x048] = BZ_KEY_KP_8;
        keyTranslationTable[0x049] = BZ_KEY_KP_9;
        keyTranslationTable[0x04E] = BZ_KEY_KP_ADD;
        keyTranslationTable[0x053] = BZ_KEY_KP_DECIMAL;
        keyTranslationTable[0x135] = BZ_KEY_KP_DIVIDE;
        keyTranslationTable[0x11C] = BZ_KEY_KP_ENTER;
        keyTranslationTable[0x059] = BZ_KEY_KP_EQUAL;
        keyTranslationTable[0x037] = BZ_KEY_KP_MULTIPLY;
        keyTranslationTable[0x04A] = BZ_KEY_KP_SUBTRACT;
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

        return keyTranslationTable[HIWORD(lParam) & 0x1FF];
    }

    static LRESULT (CALLBACK *extraWndProcFunction)(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) = nullptr;

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch(msg) {
            case WM_NCCREATE:
                //Deal with the custom data
                SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR) ((CREATESTRUCT*) lParam)->lpCreateParams);
                break;
            case WM_SIZE:
            {
                Win32Window *window = (Win32Window*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
                uint32 w = LOWORD(lParam);
                uint32 h = HIWORD(lParam);
                static_cast<D3D11Context&>(window->getGraphicsContext()).handleWindowResize(w, h);
                WindowResizeEvent event(w, h);
                window->data.width = w;
                window->data.height = h;
                window->eventCallback(event);
                break;
            }
            case WM_CLOSE:
            {
                Win32Window *window = (Win32Window*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
                WindowCloseEvent event;
                window->eventCallback(event);
                PostQuitMessage(0); //Break the event consumption ASAP
                return 0; //Don't go to DefWindowProc
            }
            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
            {
                int translatedKey = translateKey(wParam, lParam);
                if(translatedKey >= 0) {
                    Win32Window *window = (Win32Window*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
                    KeyPressedEvent event(translatedKey, LOWORD(lParam));
                    window->eventCallback(event);

                    keys[translatedKey] = true;
                }
                break;
            }
            case WM_KEYUP:
            case WM_SYSKEYUP:
            {
                int translatedKey = translateKey(wParam, lParam);
                if(translatedKey >= 0) {
                    Win32Window *window = (Win32Window*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
                    KeyReleasedEvent event(translatedKey);
                    window->eventCallback(event);

                    keys[translatedKey] = false;
                }
                break;
            }
            case WM_CHAR:
            {
                if(wParam > 32 && (wParam < 126 || wParam > 160)) {
                    Win32Window *window = (Win32Window*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
                    KeyTypedEvent event(static_cast<int>(wParam));
                    window->eventCallback(event);
                }
                break;
            }
            case WM_MOUSEMOVE:
            {
                Win32Window *window = (Win32Window*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
                MouseMovedEvent event(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
                window->eventCallback(event);
                break;
            }
            case WM_MOUSEWHEEL:
            {
                Win32Window *window = (Win32Window*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
                MouseScrolledEvent event(0.0f, (float) GET_WHEEL_DELTA_WPARAM(wParam) / (float) WHEEL_DELTA);
                window->eventCallback(event);
                break;
            }
            case WM_MOUSEHWHEEL:
            {
                Win32Window *window = (Win32Window*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
                MouseScrolledEvent event((float) GET_WHEEL_DELTA_WPARAM(wParam) / (float) WHEEL_DELTA, 0.0f);
                window->eventCallback(event);
                break;
            }
            case WM_LBUTTONDOWN:
            {
                Win32Window *window = (Win32Window*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
                MouseButtonPressedEvent event(BZ_MOUSE_BUTTON_LEFT);
                window->eventCallback(event);
                mouseButtons[BZ_MOUSE_BUTTON_LEFT] = true;
                break;
            }
            case WM_LBUTTONUP:
            {
                Win32Window *window = (Win32Window*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
                MouseButtonReleasedEvent event(BZ_MOUSE_BUTTON_LEFT);
                window->eventCallback(event);
                mouseButtons[BZ_MOUSE_BUTTON_LEFT] = false;
                break;
            }
            case WM_MBUTTONDOWN:
            {
                Win32Window *window = (Win32Window*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
                MouseButtonPressedEvent event(BZ_MOUSE_BUTTON_MIDDLE);
                window->eventCallback(event);
                mouseButtons[BZ_MOUSE_BUTTON_MIDDLE] = true;
                break;
            }
            case WM_MBUTTONUP:
            {
                Win32Window *window = (Win32Window*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
                MouseButtonReleasedEvent event(BZ_MOUSE_BUTTON_MIDDLE);
                window->eventCallback(event);
                mouseButtons[BZ_MOUSE_BUTTON_MIDDLE] = false;

                break;
            }
            case WM_RBUTTONDOWN:
            {
                Win32Window *window = (Win32Window*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
                MouseButtonPressedEvent event(BZ_MOUSE_BUTTON_RIGHT);
                window->eventCallback(event);
                mouseButtons[BZ_MOUSE_BUTTON_RIGHT] = true;

                break;
            }
            case WM_RBUTTONUP:
            {
                Win32Window *window = (Win32Window*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
                MouseButtonReleasedEvent event(BZ_MOUSE_BUTTON_RIGHT);
                window->eventCallback(event);
                mouseButtons[BZ_MOUSE_BUTTON_RIGHT] = false;

                break;
            }
            case WM_XBUTTONDOWN:
            {
                Win32Window *window = (Win32Window*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
                int button = GET_XBUTTON_WPARAM(wParam) + BZ_MOUSE_BUTTON_3;
                MouseButtonPressedEvent event(button);
                window->eventCallback(event);
                mouseButtons[button] = true;

                break;
            }
            case WM_XBUTTONUP:
            {
                Win32Window *window = (Win32Window*) GetWindowLongPtr(hWnd, GWLP_USERDATA);
                int button = GET_XBUTTON_WPARAM(wParam) + BZ_MOUSE_BUTTON_3;
                MouseButtonReleasedEvent event(button);
                window->eventCallback(event);
                mouseButtons[button] = false;

                break;
            }
            case WM_KILLFOCUS:
                keys.reset();
                mouseButtons.reset();
                break;
        }
        if(extraWndProcFunction)
            extraWndProcFunction(hWnd, msg, wParam, lParam);

        return DefWindowProc(hWnd, msg, wParam, lParam);
    }


    Win32Window::Win32Window(const WindowData &data, EventCallbackFn eventCallback) :
        Window(data, eventCallback) {
        init();
    }

    Win32Window::~Win32Window() {
        shutdown();
    }

    void Win32Window::init() {
        createKeyTranslationTable();

        BZ_LOG_CORE_INFO("Creating Win32 Window: {0}. Dimensions: ({1}, {2})", data.title, data.width, data.height);

        //register window class
        WNDCLASSEXW wndClass = {};
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
        RegisterClassExW(&wndClass);

        //create window instance
        DWORD winStyle = WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CAPTION | WS_SYSMENU | WS_SIZEBOX;
        hWnd = CreateWindowExW(
            0,
            CLASS_NAME,
            std::wstring(data.title.begin(), data.title.end()).c_str(),
            winStyle,
            CW_USEDEFAULT, CW_USEDEFAULT,
            CW_USEDEFAULT, CW_USEDEFAULT,
            //rect.right - rect.left, rect.bottom - rect.top,
            nullptr,
            nullptr,
            GetModuleHandle(nullptr),
            (LPVOID)this //Send custom data to the WndProc callback
        );
        BZ_ASSERT_CORE(hWnd, "Couldn't create window!");

        graphicsContext = std::make_unique<D3D11Context>(hWnd);
        graphicsContext->setVSync(data.vsync);

        //Adjust client region size now that we can query the window DPI
        RECT rect = {};
        rect.right = data.width;
        rect.bottom = data.height;
        ClientToScreen(hWnd, (POINT*) &rect.left);
        ClientToScreen(hWnd, (POINT*) &rect.right);

        if(isWindows10AnniversaryUpdateOrGreaterWin32())
            AdjustWindowRectExForDpi(&rect, winStyle, FALSE, 0, GetDpiForWindow(hWnd));
        else
            AdjustWindowRectEx(&rect, winStyle, FALSE, 0);

        SetWindowPos(hWnd, NULL,
                     rect.left, rect.top,
                     rect.right - rect.left, rect.bottom - rect.top,
                     SWP_NOACTIVATE | SWP_NOZORDER);

        ShowWindow(hWnd, SW_SHOWDEFAULT);
    }

    void Win32Window::shutdown() {
        UnregisterClassW(CLASS_NAME, GetModuleHandleW(nullptr));
        DestroyWindow(hWnd);
    }

    void Win32Window::pollEvents() {
        MSG msg;
        while(PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {

            //PostQuitMessage was called, just stop processing messages
            if(msg.message == WM_QUIT)
                break;

            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    void Win32Window::presentBuffer() {
        graphicsContext->presentBuffer();
    }

    void Win32Window::setExtraHandlerFunction(LRESULT(*func)(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)) {
        extraWndProcFunction = func;
    }

    void Win32Window::setTitle(const std::string &title) {
        SetWindowTextA(hWnd, title.c_str());
    }
}