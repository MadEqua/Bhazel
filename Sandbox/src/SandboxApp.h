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
    //BZ::Ref<BZ::Framebuffer> framebuffer;
    BZ::Ref<BZ::Buffer> vertexBuffer;
    BZ::Ref<BZ::Buffer> indexBuffer;
    BZ::Ref<BZ::DescriptorSet> descriptorSet;
    BZ::Ref<BZ::PipelineState> pipelineState;

    BZ::Ref<BZ::Texture2D> texture;
    BZ::Ref<BZ::TextureView> textureView;
    BZ::Ref<BZ::Sampler> sampler;

    //BZ::Ref<BZ::PerspectiveCameraController> cameraController;
};


class Sandbox : public BZ::Application {
public:
    Sandbox() {
        pushLayer(new ExampleLayer());
    }
};