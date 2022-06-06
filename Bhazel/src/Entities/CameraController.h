#pragma once

#include "Renderer/Camera.h"


namespace BZ {

struct FrameTiming;
class MouseScrolledEvent;
class WindowResizedEvent;
class Event;


template <typename T> class CameraController {
  public:
    virtual ~CameraController() = default;

    virtual void onUpdate(const FrameTiming &frameTiming) = 0;
    void onEvent(Event &e);

    T &getCamera() { return *camera; }
    const T &getCamera() const { return *camera; }

  protected:
    CameraController() = default;
    CameraController(T &camera);

    virtual bool onMouseScrolled(const MouseScrolledEvent &e) = 0;
    virtual bool onWindowResized(const WindowResizedEvent &e) = 0;

    float zoom = 1.0f;
    float cameraZoomSpeed = 0.1f;

    T *camera;
};

template <typename T> CameraController<T>::CameraController(T &camera) : camera(&camera), zoom(zoom) {
}

template <typename T> void CameraController<T>::onEvent(Event &e) {
    EventDispatcher dispatcher(e);
    dispatcher.dispatch<MouseScrolledEvent>(BZ_BIND_EVENT_FN(CameraController<T>::onMouseScrolled));
    dispatcher.dispatch<WindowResizedEvent>(BZ_BIND_EVENT_FN(CameraController<T>::onWindowResized));
}


/*-------------------------------------------------------------------------------------------*/
class CameraController2D : public CameraController<OrthographicCamera> {
  public:
    CameraController2D() = default;
    CameraController2D(OrthographicCamera &camera, float cameraMoveSpeed, bool enableRotation,
                       float cameraRotationSpeed);
    CameraController2D(OrthographicCamera &camera, float cameraMoveSpeed, bool enableRotation);

    void onUpdate(const FrameTiming &frameTiming) override;

  private:
    bool onMouseScrolled(const MouseScrolledEvent &e) override;
    bool onWindowResized(const WindowResizedEvent &e) override;

    OrthographicCamera::Parameters originalParameters;

    bool enableRotation;
    float cameraRotationSpeed = 10.0f;
    float cameraMoveSpeed = 1.0f;
};


/*-------------------------------------------------------------------------------------------*/
class FreeCameraController : public CameraController<PerspectiveCamera> {
  public:
    FreeCameraController() = default;
    FreeCameraController(PerspectiveCamera &camera, float cameraMoveSpeed);

    void onUpdate(const FrameTiming &frameTiming) override;

  private:
    bool onMouseScrolled(const MouseScrolledEvent &e) override;
    bool onWindowResized(const WindowResizedEvent &e) override;

    PerspectiveCamera::Parameters originalParameters;

    float cameraMoveSpeed = 1.0f;
    glm::ivec2 lastMousePosition = { -1, -1 };
};


/*-------------------------------------------------------------------------------------------*/
/*
 * Camera that rotates around and looks at the origin.
 * Using cilindrical coordinates to achieve that.
 */
class RotateCameraController : public CameraController<PerspectiveCamera> {
  public:
    RotateCameraController() = default;
    RotateCameraController(PerspectiveCamera &camera, float cameraMoveSpeed, float cameraRotationAccel);

    void onUpdate(const FrameTiming &frameTiming) override;

  private:
    bool onMouseScrolled(const MouseScrolledEvent &e) override;
    bool onWindowResized(const WindowResizedEvent &e) override;

    PerspectiveCamera::Parameters originalParameters;

    void recompute();

    /*
     * (r, theta, z)
     * r = cilinder radius
     * theta = angle around the cilinder circle in radians (0 is on the z axis)
     * z = height of the cilinder
     */
    glm::vec3 camPosCilindrical;

    float thetaAccel = 0.0f;
    float thetaVelocity = 0.0f;

    float zAccel = 0.0f;
    float zVelocity = 0.0f;

    float cameraRotationAccel = 1.0f;
    float cameraMoveSpeed = 1.0f;

    glm::ivec2 lastMousePosition = { -1, -1 };
};
}