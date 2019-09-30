#pragma once

#include <Bhazel.h>
#include <imgui.h>


class ExampleLayer : public BZ::Layer {
public:
    ExampleLayer();

    void onAttach() override;
    void onGraphicsContextCreated() override;

    void onUpdate(const BZ::FrameStats &frameStats) override;
    void onEvent(BZ::Event &event) override;
    void onImGuiRender(const BZ::FrameStats &frameStats) override;

private:
    //TODO: this is temporary
    BZ::ShaderLibrary shaderLibrary;

    BZ::Ref<BZ::InputDescription> inputDescription;
    BZ::Ref<BZ::Buffer> vertexBuffer;
    BZ::Ref<BZ::Buffer> indexBuffer;
    BZ::Ref<BZ::Texture> texture;

    BZ::Ref<BZ::PerspectiveCameraController> cameraController;

    //Compute shader test stuff
    //constexpr static int PARTICLE_COUNT = 1024 * 10;
    //BZ::ParticleSystem particleSystem;
};


class Sandbox : public BZ::Application {
public:
    Sandbox() {
        pushLayer(new ExampleLayer());
    }
};