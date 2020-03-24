#include "bzpch.h"

#include "Camera.h"
#include "Core/Utils.h"

#include <glm/gtc/matrix_transform.hpp>


namespace BZ {

    OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top, float near, float far) {
        parameters.left = left;
        parameters.right = right;
        parameters.bottom = bottom;
        parameters.top = top;
        parameters.near = near;
        parameters.far = far;
        computeProjectionMatrix();
    }

    void OrthographicCamera::computeProjectionMatrix() {
        projectionMatrix = Utils::ortho(parameters.left, parameters.right, parameters.bottom, parameters.top, parameters.near, parameters.far);
    }


    PerspectiveCamera::PerspectiveCamera(float fovy, float aspectRatio, float near, float far) {
        parameters.fovy = fovy;
        parameters.aspectRatio = aspectRatio;
        parameters.near = near;
        parameters.far = far;
        computeProjectionMatrix();
    }

    void PerspectiveCamera::computeProjectionMatrix() {
        projectionMatrix = Utils::perspective(parameters.fovy, parameters.aspectRatio, parameters.near, parameters.far);
    }
}