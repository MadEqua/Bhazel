#include "bzpch.h"

#ifdef BZ_PLATFORM_OPENGL43

#include "examples/imgui_impl_opengl3.h"
#include "examples/imgui_impl_glfw.h"
#include <GLFW/glfw3.h>


#elif defined BZ_PLATFORM_D3D11

#include "examples/imgui_impl_dx11.h"
#include "examples/imgui_impl_win32.h"

#include "Bhazel/Platform/Win32/Win32Window.h"
#include "Bhazel/Platform/D3D11/D3D11Context.h"
IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#endif

#include "ImGuiLayer.h"
#include "Core/Application.h"
#include "Core/Window.h"

#include <imgui.h>


namespace BZ {

    ImGuiLayer::ImGuiLayer() : 
        Layer("ImGuiLayer"),
        time(0.0f) {
    }

    void ImGuiLayer::onGraphicsContextCreated() {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsClassic();

        // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
        ImGuiStyle& style = ImGui::GetStyle();
        if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        // Setup Platform/Renderer bindings
#ifdef BZ_PLATFORM_OPENGL43
        ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow *>(Application::getInstance().getWindow().getNativeHandle()), true);
        ImGui_ImplOpenGL3_Init("#version 430");
#elif defined BZ_PLATFORM_D3D11
        Win32Window &window = static_cast<Win32Window &>(Application::getInstance().getWindow());
        ImGui_ImplWin32_Init(window.getNativeHandle());
        D3D11Context &context = static_cast<D3D11Context &>(Application::getInstance().getGraphicsContext());
        window.setExtraHandlerFunction(ImGui_ImplWin32_WndProcHandler);

        ImGui_ImplDX11_Init(context.getDevice(), context.getDeviceContext());
#endif
    }

    void ImGuiLayer::onDetach() {

#ifdef BZ_PLATFORM_OPENGL43
        ImGui_ImplGlfw_Shutdown();
        ImGui_ImplOpenGL3_Shutdown();
#elif defined BZ_PLATFORM_D3D11
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown()
#endif
        ImGui::DestroyContext();
    }

    void ImGuiLayer::onImGuiRender(const FrameStats &frameStats) {
        static bool show = true;
        ImGui::ShowDemoWindow(&show);
    }

    void ImGuiLayer::begin() {
#ifdef BZ_PLATFORM_OPENGL43
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
#elif defined BZ_PLATFORM_D3D11
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
#endif
        ImGui::NewFrame();
    }

    void ImGuiLayer::end() {
        ImGuiIO &io = ImGui::GetIO();
        Window &window = Application::getInstance().getWindow();
        io.DisplaySize = ImVec2(static_cast<float>(window.getWidth()), static_cast<float>(window.getHeight()));

        ImGui::Render();

#ifdef BZ_PLATFORM_OPENGL43
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow *backupCurrentContext = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backupCurrentContext);
        }
#elif defined BZ_PLATFORM_D3D11
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
#endif
    }
}