#pragma once

#include <Bhazel.h>
#include <memory>

#include <glm/glm.hpp>


class ExampleLayer : public BZ::Layer {
public:
    ExampleLayer();
    void onUpdate() override;
    void onEvent(BZ::Event &event) override;
    void onImGuiRender() override;


private:
    std::shared_ptr<BZ::VertexArray> vertexArray;
    std::shared_ptr<BZ::Shader> shader;
    std::shared_ptr<BZ::VertexBuffer> vertexBuffer;
    std::shared_ptr<BZ::IndexBuffer> indexBuffer;

    BZ::OrtographicCamera camera;

    glm::vec3 cameraPos;
    float cameraRot;
    bool onKeyPressedEvent(BZ::KeyPressedEvent &event);
};


class Sandbox : public BZ::Application {
public:
    Sandbox() {
        pushLayer(new ExampleLayer());
    }
};

