#pragma once

#undef near
#undef far

#include "Transform.h"


namespace BZ {

class Camera {
  public:
    virtual ~Camera() = default;

    const glm::mat4 &getProjectionMatrix() const { return projectionMatrix; }
    const glm::mat4 &getViewMatrix() const { return transform.getParentToLocalMatrix(); }

    Transform &getTransform() { return transform; }
    const Transform &getTransform() const { return transform; }

    float getExposure() const { return exposure; };
    void setExposure(float exposure) { this->exposure = exposure; };

  protected:
    Transform transform;
    glm::mat4 projectionMatrix;

    float exposure = 1.0f;
};


/*-------------------------------------------------------------------------------------------*/
class OrthographicCamera : public Camera {
  public:
    OrthographicCamera() = default;
    OrthographicCamera(float left, float right, float bottom, float top, float near = 0.0f, float far = 1.0f);

    struct Parameters {
        float left, right;
        float bottom, top;
        float near, far;
    };

    const Parameters &getParameters() const { return parameters; }
    void setParameters(const Parameters &parameters) {
        this->parameters = parameters;
        computeProjectionMatrix();
    }

  private:
    Parameters parameters;

    void computeProjectionMatrix();
};


/*-------------------------------------------------------------------------------------------*/
class PerspectiveCamera : public Camera {
  public:
    PerspectiveCamera() = default;
    PerspectiveCamera(float fovy, float aspectRatio, float near = 0.1f, float far = 100.0f);

    struct Parameters {
        float fovy;
        float aspectRatio;
        float near, far;
    };

    const Parameters &getParameters() const { return parameters; }
    void setParameters(const Parameters &parameters) {
        this->parameters = parameters;
        computeProjectionMatrix();
    }

    // Near left bottom, near right bottom, near left top, near right top, ...
    // const glm::vec3* getFrustumCornerPoints() const;

  private:
    Parameters parameters;
    // mutable glm::vec3 frustumCornerPoints[8];

    void computeProjectionMatrix();
};
}