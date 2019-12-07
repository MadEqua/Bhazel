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
    BZ::OrthographicCameraController cameraController;

    BZ::Ref<BZ::Texture2D> tex1;
    BZ::Ref<BZ::Texture2D> tex2;

    BZ::ParticleSystem2D particleSystem;
};


class Sandbox : public BZ::Application {
public:
    Sandbox() {
        pushLayer(new ExampleLayer());
    }
};