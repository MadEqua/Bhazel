#pragma once

#undef near
#undef far

#include "Transform.h"


namespace BZ {

    class Camera {
    public:
        virtual ~Camera() = default;

        const glm::mat4& getProjectionMatrix() const { return projectionMatrix; }
        const glm::mat4& getViewMatrix() const { return transform.getParentToLocalMatrix(); }

        Transform& getTransform() { return transform; }
        const Transform& getTransform() const { return transform; }

    protected:
        Transform transform;
        glm::mat4 projectionMatrix;
    };


    //Meant for 2D rendering.
    class OrthographicCamera : public Camera {
    public:
        OrthographicCamera();
        OrthographicCamera(float left, float right, float bottom, float top, float near = 0.0f, float far = 1.0f);
        
        float getRotation() const { return transform.getRotationEuler().z; }
        void setRotation(float rotDeg) { transform.setRotationEuler(0.0f, 0.0f, rotDeg); }

        struct Parameters {
            float left, right;
            float bottom, top;
            float near, far;
        };

        Parameters& getParameters() { return parameters;  }
        void setParameters(const Parameters &parameters) { this->parameters = parameters; computeProjectionMatrix(); }

    private:
        Parameters parameters;

        void computeProjectionMatrix();
    };


    class PerspectiveCamera : public Camera {
    public:
        PerspectiveCamera();
        PerspectiveCamera(float fovy, float aspectRatio, float near = 0.1f, float far = 100.0f);
        
        struct Parameters {
            float fovy;
            float aspectRatio;
            float near, far;
        };

        Parameters& getParameters() { return parameters; }
        void setParameters(const Parameters &parameters) { this->parameters = parameters; computeProjectionMatrix(); }

    private:
        Parameters parameters;

        void computeProjectionMatrix();
    };
}