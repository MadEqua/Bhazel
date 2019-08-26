#pragma once

#include <Bhazel.h>
#include <glm/glm.hpp>
#include <ImGui/imgui.h>


class ExampleLayer : public BZ::Layer {
public:
    ExampleLayer();
    void onUpdate(BZ::Timestep timestep) override;
    void onEvent(BZ::Event &event) override;
    void onImGuiRender() override;


private:
    BZ::Ref<BZ::VertexArray> vertexArray;
    BZ::Ref<BZ::Shader> shader;
    BZ::Ref<BZ::VertexBuffer> vertexBuffer;
    BZ::Ref<BZ::IndexBuffer> indexBuffer;
    BZ::Ref<BZ::Texture> texture;

    BZ::OrtographicCamera camera;

    glm::vec3 cameraPos;
    float cameraRot;

    bool onKeyPressedEvent(BZ::KeyPressedEvent &event);
};


class EventTestLayer : public BZ::Layer {
public:
    void onUpdate(BZ::Timestep timestep) {
        if(BZ::Input::isKeyPressed(BZ_KEY_F)) BZ_LOG_TRACE("F");
        if(BZ::Input::isKeyPressed(BZ_KEY_F4)) BZ_LOG_TRACE("F4");
        if(BZ::Input::isKeyPressed(BZ_KEY_TAB)) BZ_LOG_TRACE("TAB");
        if(BZ::Input::isKeyPressed(BZ_KEY_LEFT_ALT)) BZ_LOG_TRACE("LEFT_ALT");
        if(BZ::Input::isKeyPressed(BZ_KEY_H)) BZ_LOG_TRACE("H");

        if(BZ::Input::isMouseButtonPressed(BZ_MOUSE_BUTTON_LEFT)) BZ_LOG_TRACE("BZ_MOUSE_BUTTON_LEFT");
        if(BZ::Input::isMouseButtonPressed(BZ_MOUSE_BUTTON_RIGHT)) BZ_LOG_TRACE("BZ_MOUSE_BUTTON_RIGHT");
        if(BZ::Input::isMouseButtonPressed(BZ_MOUSE_BUTTON_MIDDLE)) BZ_LOG_TRACE("BZ_MOUSE_BUTTON_MIDDLE");
        if(BZ::Input::isMouseButtonPressed(BZ_MOUSE_BUTTON_4)) BZ_LOG_TRACE("BZ_MOUSE_BUTTON_4");
        if(BZ::Input::isMouseButtonPressed(BZ_MOUSE_BUTTON_5)) BZ_LOG_TRACE("BZ_MOUSE_BUTTON_5");

        //auto pos = BZ::Input::getMousePosition();
        //BZ_LOG_TRACE("({0},{1})", pos.first, pos.second);
    }
    void onEvent(BZ::Event &event) override {
        BZ_LOG_TRACE(event);
    }
    void onImGuiRender() {
        ImGui::Begin("Test");
        ImGui::LabelText("Hello!", "");
        ImGui::End();
    }
};

class Sandbox : public BZ::Application {
public:
    Sandbox() {
        //pushLayer(new ExampleLayer());
        pushLayer(new EventTestLayer());
    }
};

