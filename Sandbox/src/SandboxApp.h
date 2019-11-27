#pragma once

#include <Bhazel.h>


class ExampleLayer : public BZ::Layer {
public:
    ExampleLayer();

    void onAttach() override;
    void onGraphicsContextCreated() override;

    void onUpdate(const BZ::FrameStats &frameStats) override;
    void onEvent(BZ::Event &event) override;
    void onImGuiRender(const BZ::FrameStats &frameStats) override;

private:
    static const uint32 OBJECT_COUNT = 1000;
    
    struct Object {
        glm::vec2 pos;
        glm::vec2 vel;
        glm::vec2 dims;
        glm::vec3 tint;
        float rot;
        uint32 texId;
    };

    Object objects[OBJECT_COUNT];

    BZ::OrthographicCameraController cameraController;
    glm::vec2 pos = {};
    float rot = 0.0f;
    BZ::Ref<BZ::Texture2D> tex1;
    BZ::Ref<BZ::Texture2D> tex2;
};


class Sandbox : public BZ::Application {
public:
    Sandbox() {
        pushLayer(new ExampleLayer());
    }
};