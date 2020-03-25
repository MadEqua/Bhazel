#pragma once

#include <Bhazel.h>


class ParticleLayer : public BZ::Layer {
public:
    ParticleLayer();

    void onAttach() override;
    void onGraphicsContextCreated() override;

    void onUpdate(const BZ::FrameStats &frameStats) override;
    void onEvent(BZ::Event &event) override;
    void onImGuiRender(const BZ::FrameStats &frameStats) override;

private:
    BZ::OrthographicCamera camera;
    BZ::OrthographicCameraController cameraController;

    BZ::Ref<BZ::Texture2D> tex1;
    BZ::Ref<BZ::Texture2D> tex2;

    BZ::ParticleSystem2D particleSystem;
};


class Layer3D : public BZ::Layer {
public:
    Layer3D();

    void onAttach() override;
    void onGraphicsContextCreated() override;

    void onUpdate(const BZ::FrameStats &frameStats) override;
    void onEvent(BZ::Event &event) override;
    void onImGuiRender(const BZ::FrameStats &frameStats) override;

private:
    BZ::Transform cubeTransforms[2];

    BZ::PerspectiveCamera camera;
    BZ::RotateCameraController cameraController;
};



class Sandbox : public BZ::Application {
public:
    Sandbox() {
        //pushLayer(new ParticleLayer());
        pushLayer(new Layer3D());
    }
};