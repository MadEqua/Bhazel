#include <Bhazel.h>

#include "imgui/imgui.h"

class ExampleLayer : public BZ::Layer {
public:
    ExampleLayer() : Layer("Example") {
    }

    void onUpdate() override {
    }

    void onEvent(BZ::Event &event) override {
    }

    void onImGuiRender() override {
        ImGui::Begin("Test");
        ImGui::Text("Hello");
        ImGui::End();
    }
};

class Sandbox : public BZ::Application {
public:
    Sandbox() {
    pushLayer(new ExampleLayer());
    }
};


BZ::Application* BZ::createApplication() {
    return new Sandbox();
}