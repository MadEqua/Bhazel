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

    /*const glm::vec3* PerspectiveCamera::getFrustumCornerPoints() const {
        float tg = glm::tan(glm::radians(parameters.fovy * 0.5f));

        glm::vec2 nearDims = { tg * parameters.near * 2.0f * parameters.aspectRatio, tg * parameters.near * 2.0f };
        frustumCornerPoints[0].x = -nearDims.x * 0.5f;
        frustumCornerPoints[0].y = -nearDims.y * 0.5f;
        frustumCornerPoints[0].z = -parameters.near;

        frustumCornerPoints[1].x = nearDims.x * 0.5f;
        frustumCornerPoints[1].y = -nearDims.y * 0.5f;
        frustumCornerPoints[1].z = -parameters.near;

        frustumCornerPoints[2].x = -nearDims.x * 0.5f;
        frustumCornerPoints[2].y = nearDims.y * 0.5f;
        frustumCornerPoints[2].z = -parameters.near;

        frustumCornerPoints[3].x = nearDims.x * 0.5f;
        frustumCornerPoints[3].y = nearDims.y * 0.5f;
        frustumCornerPoints[3].z = -parameters.near;

        glm::vec2 farDims = { tg * parameters.far * 2.0f * parameters.aspectRatio, tg * parameters.far * 2.0f };
        frustumCornerPoints[4].x = -farDims.x * 0.5f;
        frustumCornerPoints[4].y = -farDims.y * 0.5f;
        frustumCornerPoints[4].z = -parameters.far;

        frustumCornerPoints[5].x = farDims.x * 0.5f;
        frustumCornerPoints[5].y = -farDims.y * 0.5f;
        frustumCornerPoints[5].z = -parameters.far;

        frustumCornerPoints[6].x = -farDims.x * 0.5f;
        frustumCornerPoints[6].y = farDims.y * 0.5f;
        frustumCornerPoints[6].z = -parameters.far;

        frustumCornerPoints[7].x = farDims.x * 0.5f;
        frustumCornerPoints[7].y = farDims.y * 0.5f;
        frustumCornerPoints[7].z = -parameters.far;

        for (int i = 0; i < 8; ++i) {
            frustumCornerPoints[i] = transform.getLocalToParentMatrix() * glm::vec4(frustumCornerPoints[i], 1.0f);
        }

        return frustumCornerPoints;
    }*/

    void PerspectiveCamera::computeProjectionMatrix() {
        projectionMatrix = Utils::perspective(parameters.fovy, parameters.aspectRatio, parameters.near, parameters.far);
    }
}