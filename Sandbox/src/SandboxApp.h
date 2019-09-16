#pragma once

#include <Bhazel.h>
#include <ImGui/imgui.h>


class ExampleLayer : public BZ::Layer {
public:
    ExampleLayer();

    void onAttach() override;
    void onGraphicsContextCreated() override;

    void onUpdate(BZ::TimeDuration deltaTime) override;
    void onEvent(BZ::Event &event) override;
    void onImGuiRender(BZ::TimeDuration deltaTime) override;

private:
    //TODO: this is temporary
    BZ::ShaderLibrary shaderLibrary;

    BZ::Ref<BZ::InputDescription> inputDescription;
    BZ::Ref<BZ::Buffer> vertexBuffer;
    BZ::Ref<BZ::Buffer> indexBuffer;
    BZ::Ref<BZ::Texture> texture;

    BZ::Ref <BZ::OrtographicCamera> camera;

    glm::vec2 cameraPos;
    float cameraRot;

    glm::vec3 pos;
    glm::vec3 disp;

    //Compute shader test stuff
    constexpr static int PARTICLE_COUNT = 1024 * 32;
    constexpr static int WORK_GROUP_SIZE = 256;

    struct Particle {
        glm::vec4 pos;
        glm::vec4 col;
    };

    BZ::Ref<BZ::Buffer> particlesBuffer;
    BZ::Ref<BZ::InputDescription> particlesInputDescription;

    bool onWindowResizeEvent(BZ::WindowResizeEvent &ev);
};


class Sandbox : public BZ::Application {
public:
    Sandbox() {
        pushLayer(new ExampleLayer());
    }
};