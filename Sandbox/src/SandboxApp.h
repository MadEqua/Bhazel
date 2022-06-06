#pragma once

#include <Bhazel.h>


class ParticleLayer : public BZ::Layer {
  public:
    ParticleLayer();

    void onAttachToEngine() override;

    void onUpdate(const BZ::FrameTiming &frameTiming) override;
    void onEvent(BZ::Event &event) override;
    void onImGuiRender(const BZ::FrameTiming &frameTiming) override;

  private:
    // BZ::OrthographicCamera camera;
    // BZ::CameraController2D cameraController;

    // BZ::ParticleSystem2D particleSystem;
};


// class Layer3D : public BZ::Layer {
//  public:
//    Layer3D();
//
//    void onAttachToEngine() override;
//
//    void onUpdate(const BZ::FrameTiming &frameTiming) override;
//    void onEvent(BZ::Event &event) override;
//    void onImGuiRender(const BZ::FrameTiming &frameTiming) override;
//
//  private:
//    BZ::Scene scenes[3];
//    int activeScene = 0;
//
//    BZ::PerspectiveCamera camera;
//    BZ::RotateCameraController rotateCameraController;
//    BZ::FreeCameraController freeCameraController;
//    bool useFreeCamera = false;
//
//    BZ::OrthographicCamera orthoCamera;
//};


class TestScene : public BZ::Scene {
  public:
    TestScene() {
        //pushLayer(new ParticleLayer());
        pushLayer(new Layer3D());
    }

    virtual ~TestScene() = default;

    void onAttachToEngine() override;
    void onUpdate(const BZ::FrameTiming &frameTiming) override;

  private:
    BZ::Ref<BZ::Texture2D> tex1;
    BZ::Ref<BZ::Texture2D> tex2;

    entt::entity emitterEnt;
};


class Sandbox : public BZ::Application {
  public:
    Sandbox() {
        settings.enable2dRenderer = true;
        settings.enable3dRenderer = false;
        settings.enableImGuiRenderer = true;
        sceneManager.addScene(new TestScene());
    }
};