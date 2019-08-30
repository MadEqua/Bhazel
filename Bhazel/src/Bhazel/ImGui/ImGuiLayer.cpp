#include "bzpch.h"
#include "ImGuiLayer.h"

#include "Bhazel/Application.h"

#include <imgui.h>

#include "examples/imgui_impl_opengl3.h"
#include "examples/imgui_impl_glfw.h"

#include "Bhazel/Platform/Win32/Win32Window.h"
#include "Bhazel/Platform/D3D11/D3D11Context.h"
#include "examples/imgui_impl_dx11.h"
#include "examples/imgui_impl_win32.h"
IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#include <GLFW/glfw3.h>

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
        if(Renderer::api == Renderer::API::OpenGL) {
            ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(Application::getInstance().getWindow().getNativeWindowHandle()), true);
            ImGui_ImplOpenGL3_Init("#version 430");
        }
        else if(Renderer::api == Renderer::API::D3D11) {
            Win32Window &window = static_cast<Win32Window&>(Application::getInstance().getWindow());
            ImGui_ImplWin32_Init(window.getNativeWindowHandle());
            D3D11Context &context = static_cast<D3D11Context&>(window.getGraphicsContext());
            window.setExtraHandlerFunction(ImGui_ImplWin32_WndProcHandler);

            ImGui_ImplDX11_Init(context.getDevice(), context.getDeviceContext());
        }
    }

    void ImGuiLayer::onDetach() {
        if(Renderer::api == Renderer::API::OpenGL) {
            ImGui_ImplGlfw_Shutdown();
            ImGui_ImplOpenGL3_Shutdown();
        }
        else if(Renderer::api == Renderer::API::D3D11) {
            ImGui_ImplDX11_Shutdown();
            ImGui_ImplWin32_Shutdown();
        }

        ImGui::DestroyContext();
    }

    void ImGuiLayer::onImGuiRender() {
        static bool show = true;
        ImGui::ShowDemoWindow(&show);
    }

    void ImGuiLayer::begin() {
        if(Renderer::api == Renderer::API::OpenGL) {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
        }
        else if(Renderer::api == Renderer::API::D3D11) {
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
        }
        ImGui::NewFrame();
    }

    void ImGuiLayer::end() {
        ImGuiIO &io = ImGui::GetIO();
        Window &window = Application::getInstance().getWindow();
        io.DisplaySize = ImVec2(static_cast<float>(window.getWidth()), static_cast<float>(window.getHeight()));

        ImGui::Render();

        if(Renderer::api == Renderer::API::OpenGL) {
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                GLFWwindow* backupCurrentContext = glfwGetCurrentContext();
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
                glfwMakeContextCurrent(backupCurrentContext);
            }
        }
        else if(Renderer::api == Renderer::API::D3D11) {
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
            if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
            }
        }
    }
}