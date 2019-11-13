#pragma once

#include "Renderer/Camera.h"


namespace BZ {

    struct FrameStats;
    class MouseScrolledEvent;
    class WindowResizedEvent;
    class Event;


    template<typename T>
    class CameraController {
    public:
        virtual ~CameraController() = default;

        virtual void onUpdate(const FrameStats &frameStats) = 0;
        void onEvent(Event &e);

        T& getCamera() { return camera; }
        const T& getCamera() const { return camera; }

    protected:
        CameraController(T &camera, float zoom);

        virtual bool onMouseScrolled(MouseScrolledEvent &e) = 0;
        virtual bool onWindowResized(WindowResizedEvent &e) = 0;

        float zoom;

        float cameraMoveSpeed = 200.0f;
        float cameraZoomSpeed = 0.1f;

        T camera;
    };

    template<typename T>
    CameraController<T>::CameraController(T &camera, float zoom) :
        camera(camera), zoom(zoom) {
    }

    template<typename T>
    void CameraController<T>::onEvent(Event &e) {
        EventDispatcher dispatcher(e);
        dispatcher.dispatch<MouseScrolledEvent>(BZ_BIND_EVENT_FN(CameraController<T>::onMouseScrolled));
        dispatcher.dispatch<WindowResizedEvent>(BZ_BIND_EVENT_FN(CameraController<T>::onWindowResized));
    }


    class OrthographicCameraController : public CameraController<OrthographicCamera> {
    public:
        OrthographicCameraController();
        OrthographicCameraController(float left, float right, float bottom, float top, float near = 0.0f, float far = 1.0f, bool enableRotation = true);

        void onUpdate(const FrameStats &frameStats) override;

    private:
        bool onMouseScrolled(MouseScrolledEvent &e) override;
        bool onWindowResized(WindowResizedEvent &e) override;

        bool enableRotation;
        float cameraRotationSpeed = 90.0f;
        float originalLeft, originalRight, originalBottom, originalTop;
        float left, right, bottom, top, near, far;
    };


    class PerspectiveCameraController : public CameraController<PerspectiveCamera> {
    public:
        PerspectiveCameraController();
        PerspectiveCameraController(float fovy, float aspectRatio);

        void onUpdate(const FrameStats &frameStats) override;

    private:
        bool onMouseScrolled(MouseScrolledEvent &e) override;
        bool onWindowResized(WindowResizedEvent &e) override;

        float fovy;
        float aspectRatio;
        glm::ivec2 lastMousePosition = {-1, -1};
        glm::ivec2 windowSize;
    };
}