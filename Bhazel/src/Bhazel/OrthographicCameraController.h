#pragma once

#include "Bhazel/Renderer/OrthographicCamera.h"


namespace BZ {

    struct FrameStats;
    class MouseScrolledEvent;
    class WindowResizeEvent;
    class Event;

    class OrthographicCameraController
    {
    public:
        OrthographicCameraController(float aspectRatio, float zoom = 1.0f, bool enableRotation = true);

        void onUpdate(const FrameStats &frameStats);
        void onEvent(Event &e);

        OrthographicCamera& getCamera() { return camera; }
        const OrthographicCamera& getCamera() const { return camera; }

    private:
        bool onMouseScrolled(MouseScrolledEvent &e);
        bool onWindowResized(WindowResizeEvent &e);

        float aspectRatio;
        float zoom;
        bool enableRotation;

        float cameraMoveSpeed = 2.0f;
        float cameraRotationSpeed = 180.0f;
        float cameraZoomSpeed = 0.05f;

        OrthographicCamera camera;
    };
}