#include "bzpch.h"

#include "OrtographicCamera.h"
#include  <glm/gtc/matrix_transform.hpp>


namespace BZ {

    OrtographicCamera::OrtographicCamera(float left, float right, float bottom, float top) :
        OrtographicCamera(left, right, bottom, top, -1.0f, 1.0f) {
    }

    OrtographicCamera::OrtographicCamera(float left, float right, float bottom, float top, float n, float f) :
        position(0.0f), rotation(0.0f),
            projectionMatrix(glm::ortho(left, right, bottom, top, n, f)) {
            computeViewMatrix();
    }

    void OrtographicCamera::computeViewMatrix() {
        glm::mat4 iden(1.0f);
        viewMatrix = glm::rotate(iden, -glm::radians(rotation), glm::vec3(0, 0, 1)) * glm::translate(iden, -glm::vec3(position, 0.0f));
        viewProjectionMatrix = projectionMatrix * viewMatrix;
    }
}