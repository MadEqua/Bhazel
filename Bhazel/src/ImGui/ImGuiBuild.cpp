#include "bzpch.h"

#ifdef BZ_PLATFORM_OPENGL43

#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include "examples/imgui_impl_opengl3.cpp"
#include "examples/imgui_impl_glfw.cpp"

#elif defined BZ_PLATFORM_D3D11

#include "examples/imgui_impl_dx11.cpp"
#include "examples/imgui_impl_win32.cpp"

#endif