#include "bzpch.h"

#include "OrthographicCamera.h"
#include "Bhazel/Core/Utils.h"

#include <glm/gtc/matrix_transform.hpp>


namespace BZ {

    OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top, float near, float far) :
        position(0.0f), rotation(0.0f) {
        computeProjectionMatrix(left, right, bottom, top, near, far);
        computeViewMatrix();
    }

    void OrthographicCamera::computeProjectionMatrix(float left, float right, float bottom, float top, float near, float far) {
        projectionMatrix = Utils::ortho(left, right, bottom, top, near, far);
    }

    void OrthographicCamera::computeViewMatrix() {
        glm::mat4 iden(1.0f);
        viewMatrix = glm::rotate(iden, glm::radians(-rotation), glm::vec3(0, 0, 1));
        viewMatrix = glm::translate(viewMatrix, glm::vec3(-position, 0.0f));
        viewProjectionMatrix = projectionMatrix * viewMatrix;
    }
}