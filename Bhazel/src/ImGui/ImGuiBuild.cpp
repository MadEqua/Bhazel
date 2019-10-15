#include "bzpch.h"

#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include "examples/imgui_impl_opengl3.cpp"
#include "examples/imgui_impl_glfw.cpp"


//Avoid name collisions with both implementations
#define g_Time g_Time_Win32
#define g_WantUpdateMonitors g_WantUpdateMonitors_Win32
#define ImGui_ImplWin32_SetImeInputPos ImGui_ImplWin32_SetImeInputPos_Win32

#include "examples/imgui_impl_dx11.cpp"
#include "examples/imgui_impl_win32.cpp"
