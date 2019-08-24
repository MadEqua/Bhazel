#pragma once

#include <Bhazel.h>
#include <glm/glm.hpp>


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
    void onUpdate(BZ::Timestep timestep) {}
    void onEvent(BZ::Event &event) override {
        BZ_LOG_TRACE(event);
    }
    void onImGuiRender() {}
};

class Sandbox : public BZ::Application {
public:
    Sandbox() {
        //pushLayer(new ExampleLayer());
        pushLayer(new EventTestLayer());
    }
};

