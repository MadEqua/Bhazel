#include "bzpch.h"
#include "ImGuiLayer.h"

#include "Bhazel/Application.h"
#include "Bhazel/Events/MouseEvent.h"
#include "Bhazel/Events/KeyEvent.h"
#include "Bhazel/Events/ApplicationEvent.h"

#include "imgui.h"
#include "Bhazel/Platform/OpenGL/ImGuiOpenGLRenderer.h"
#include "GLFW/glfw3.h"

namespace BZ {

    ImGuiLayer::ImGuiLayer() : 
        Layer("ImGuiLayer"),
        time(0.0f) {
    }

    ImGuiLayer::~ImGuiLayer() {
    }

    void ImGuiLayer::onAttach() {
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGuiIO &io = ImGui::GetIO();
        io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
        io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

        //TEMPORARY
        io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
        io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
        io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
        io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
        io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
        io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
        io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
        io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
        io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
        io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
        io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
        io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
        io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
        io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
        io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
        io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
        io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
        io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
        io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
        io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
        io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

        ImGui_ImplOpenGL3_Init("#version 410");
    }

    void ImGuiLayer::onDetach() {
    }

    void ImGuiLayer::onUpdate() {
        ImGuiIO &io = ImGui::GetIO();
        Application &app = Application::getInstance();
        io.DisplaySize = ImVec2(app.getWindow().getWidth(), app.getWindow().getHeight());

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        float time = glfwGetTime();
        io.DeltaTime = this->time > 0.0 ? (time - this->time) : (1.0f / 60.0f);
        this->time = time;

        static bool show = true;
        ImGui::ShowDemoWindow(&show);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void ImGuiLayer::onEvent(Event &event) {
        ImGuiIO &io = ImGui::GetIO();

        EventDispatcher dispatcher(event);
        dispatcher.dispatch<MouseButtonPressedEvent>(BZ_BIND_EVENT_FN(ImGuiLayer::onMouseButtonPressedEvent));
        dispatcher.dispatch<MouseButtonReleasedEvent>(BZ_BIND_EVENT_FN(ImGuiLayer::onMouseButtonReleasedEvent));
        dispatcher.dispatch<MouseMovedEvent>(BZ_BIND_EVENT_FN(ImGuiLayer::onMouseMovedEvent));
        dispatcher.dispatch<MouseScrolledEvent>(BZ_BIND_EVENT_FN(ImGuiLayer::onMouseScrolledEvent));

        dispatcher.dispatch<KeyPressedEvent>(BZ_BIND_EVENT_FN(ImGuiLayer::onKeyPressedEvent));
        dispatcher.dispatch<KeyReleasedEvent>(BZ_BIND_EVENT_FN(ImGuiLayer::onKeyReleasedEvent));
        dispatcher.dispatch<KeyTypedEvent>(BZ_BIND_EVENT_FN(ImGuiLayer::onKeyTypedEvent));

        dispatcher.dispatch<WindowResizeEvent>(BZ_BIND_EVENT_FN(ImGuiLayer::onWindowResizedEvent));
    }

    bool ImGuiLayer::onMouseButtonPressedEvent(MouseButtonPressedEvent &e) {
        ImGuiIO &io = ImGui::GetIO();
        io.MouseDown[e.getMouseButton()] = true;
        return false;
    }

    bool ImGuiLayer::onMouseButtonReleasedEvent(MouseButtonReleasedEvent &e) {
        ImGuiIO &io = ImGui::GetIO();
        io.MouseDown[e.getMouseButton()] = false;
        return false;
    }

    bool ImGuiLayer::onMouseMovedEvent(MouseMovedEvent &e) {
        ImGuiIO &io = ImGui::GetIO();
        io.MousePos = ImVec2(e.getX(), e.getY());
        return false;
    }

    bool ImGuiLayer::onMouseScrolledEvent(MouseScrolledEvent &e) {
        ImGuiIO &io = ImGui::GetIO();
        io.MouseWheelH += (float) e.getXOffset();
        io.MouseWheel += (float) e.getYOffset();
        return false;
    }

    bool ImGuiLayer::onKeyPressedEvent(KeyPressedEvent &e) {
        ImGuiIO &io = ImGui::GetIO();
        
        io.KeysDown[e.getKeyCode()] = true;

        io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
        io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
        io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
        io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
        return false;
    }

    bool ImGuiLayer::onKeyReleasedEvent(KeyReleasedEvent &e) {
        ImGuiIO &io = ImGui::GetIO();

        io.KeysDown[e.getKeyCode()] = false;

        io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
        io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
        io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
        io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
        return false;
    }

    bool ImGuiLayer::onKeyTypedEvent(KeyTypedEvent &e) {
        ImGuiIO &io = ImGui::GetIO();

        if(e.getKeyCode() > 0 && e.getKeyCode() < 0x10000) {
            io.AddInputCharacter((unsigned short) e.getKeyCode());
        }

        return false;
    }

    bool ImGuiLayer::onWindowResizedEvent(WindowResizeEvent &e) {
        ImGuiIO &io = ImGui::GetIO();
        io.DisplaySize = ImVec2(e.getWidth(), e.getHeight());
        io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
        //glViewport(0.0f, 0.0f, e.getWidth(), e.getHeight());
        return false;
    }
}