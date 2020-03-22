#include "bzpch.h"

#include "Camera.h"
#include "Core/Utils.h"

#include <glm/gtc/matrix_transform.hpp>


namespace BZ {

    OrthographicCamera::OrthographicCamera() {
        computeProjectionMatrix(-1, 1, -1, 1, 0, 1);
        computeViewMatrix();
    }

    OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top, float near, float far) {
        computeProjectionMatrix(left, right, bottom, top, near, far);
        computeViewMatrix();
    }

    void OrthographicCamera::computeProjectionMatrix(float left, float right, float bottom, float top, float near, float far) {
        projectionMatrix = Utils::ortho(left, right, bottom, top, near, far);
        viewProjectionMatrix = projectionMatrix * viewMatrix;
    }

    void OrthographicCamera::computeViewMatrix() {
        glm::mat4 iden(1.0f);
        viewMatrix = glm::rotate(iden, glm::radians(-rotationDeg), glm::vec3(0, 0, 1));
        viewMatrix = glm::translate(viewMatrix, -position);
        viewProjectionMatrix = projectionMatrix * viewMatrix;
    }


    PerspectiveCamera::PerspectiveCamera() {
        computeProjectionMatrix(50.0f, 16.0f/10.0f, 0.1f, 100.0f);
        computeViewMatrix();
    }

    PerspectiveCamera::PerspectiveCamera(float fovy, float aspectRatio, float near, float far) {
        computeProjectionMatrix(fovy, aspectRatio, near, far);
        computeViewMatrix();
    }

    void PerspectiveCamera::computeProjectionMatrix(float fovy, float aspectRatio, float near, float far) {
        projectionMatrix = Utils::perspective(fovy, aspectRatio, near, far);
        viewProjectionMatrix = projectionMatrix * viewMatrix;
    }

    void PerspectiveCamera::computeViewMatrix() {
        glm::mat4 iden(1.0f);
        viewMatrix = glm::rotate(iden, glm::radians(-eulerRotation.y), glm::vec3(0, 1, 0));
        viewMatrix = glm::rotate(viewMatrix, glm::radians(-eulerRotation.x), glm::vec3(1, 0, 0));
        viewMatrix = glm::rotate(viewMatrix, glm::radians(-eulerRotation.z), glm::vec3(0, 0, 1));
        viewMatrix = glm::translate(viewMatrix, -position);

        viewProjectionMatrix = projectionMatrix * viewMatrix;
    }
}