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
    BZ::Ref<BZ::Framebuffer> framebuffer;
    BZ::Ref<BZ::Buffer> vertexBuffer;
    BZ::Ref<BZ::Buffer> indexBuffer;
    BZ::Ref<BZ::Buffer> constantBuffer;
    BZ::Ref<BZ::DescriptorSet> descriptorSet;
    BZ::Ref<BZ::PipelineState> pipelineState;

    //BZ::Ref<BZ::CommandBuffer> buffers[BZ::MAX_FRAMES_IN_FLIGHT];

    BZ::Ref<BZ::PerspectiveCameraController> cameraController;

    struct ConstantData {
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 proj = glm::mat4(1.0f);
    } constantData;
};


class Sandbox : public BZ::Application {
public:
    Sandbox() {
        pushLayer(new ExampleLayer());
    }
};