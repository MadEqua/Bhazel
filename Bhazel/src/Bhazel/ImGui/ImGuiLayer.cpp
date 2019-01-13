#include "bzpch.h"
#include "ImGuiLayer.h"
#include "Bhazel/Application.h"
#include "Bhazel/Events/MouseEvent.h"
#include "Bhazel/Events/KeyEvent.h"

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

        if(event.getEventType() == EventType::MouseMoved) {
            MouseMovedEvent &castedEvent = static_cast<MouseMovedEvent&>(event);
            io.MousePos = ImVec2(castedEvent.getX(), castedEvent.getY());
        }
        else if(event.getEventType() == EventType::MouseButtonPressed) {
            MouseButtonPressedEvent &castedEvent = static_cast<MouseButtonPressedEvent&>(event);
            io.MouseDown[castedEvent.getMouseButton()] = true;
        }
        else if(event.getEventType() == EventType::MouseButtonReleased) {
            MouseButtonReleasedEvent &castedEvent = static_cast<MouseButtonReleasedEvent&>(event);
            io.MouseDown[castedEvent.getMouseButton()] = false;
        }
        else if(event.getEventType() == EventType::MouseScrolled) {
            MouseScrolledEvent &castedEvent = static_cast<MouseScrolledEvent&>(event);
            io.MouseWheelH += (float) castedEvent.getXOffset();
            io.MouseWheel += (float) castedEvent.getYOffset();
        }
        else if(event.getEventType() == EventType::KeyPressed) {
            KeyPressedEvent &castedEvent = static_cast<KeyPressedEvent&>(event);
            io.KeysDown[castedEvent.getKeyCode()] = true;

            io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
            io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
            io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
            io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];

            if(castedEvent.getKeyCode() > 0 && castedEvent.getKeyCode() < 0x10000) {
                io.AddInputCharacter((unsigned short) castedEvent.getKeyCode());
            }
        }
        else if(event.getEventType() == EventType::KeyReleased) {
            KeyReleasedEvent &castedEvent = static_cast<KeyReleasedEvent&>(event);
            io.KeysDown[castedEvent.getKeyCode()] = false;

            io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
            io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
            io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
            io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
        }

        event.handled = true;
    }
}