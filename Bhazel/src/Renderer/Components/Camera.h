#pragma once

#undef near
#undef far


namespace BZ {

// TODO: dirty flag?
struct Camera {
    glm::mat4 projectionMatrix;
    float exposure = 1.0f;
};

struct OrthographicCamera : public Camera {
    OrthographicCamera(float left, float right, float bottom, float top, float near = 0.0f, float far = 1.0f);

    float left, right;
    float bottom, top;
    float near, far;
};

struct PerspectiveCamera : public Camera {
    PerspectiveCamera(float fovy, float aspectRatio, float near = 0.1f, float far = 100.0f);

    float fovy;
    float aspectRatio;
    float near, far;
};
}