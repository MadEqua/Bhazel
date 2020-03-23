#include "bzpch.h"

#include "Transform.h"
#include <glm/gtc/matrix_transform.hpp>


namespace BZ {

    Transform::Transform() {
        computeMatrices();
    }

    Transform::Transform(const glm::vec3 &translation) :
        translation(translation) {
        computeMatrices();
    }

    Transform::Transform(const glm::vec3 &translation, const glm::vec3 &rotationEuler, const glm::vec3 &scale) :
        translation(translation),
        rotationEuler(rotationEuler),
        scale(scale) {
        computeMatrices();
    }

    void Transform::lookAt(const glm::vec3 &point) {
        glm::vec3 z = glm::normalize(translation - point);
        glm::vec3 y(0.0f, 1.0f, 0.0f);
        glm::vec3 x = glm::normalize(glm::cross(y, z));
        y = glm::normalize(glm::cross(z, x));

        localToParentMatrix[0][0] = x.x;
        localToParentMatrix[0][1] = x.y;
        localToParentMatrix[0][2] = x.z;
        localToParentMatrix[0][3] = 0.0f;

        localToParentMatrix[1][0] = y.x;
        localToParentMatrix[1][1] = y.y;
        localToParentMatrix[1][2] = y.z;
        localToParentMatrix[1][3] = 0.0f;

        localToParentMatrix[2][0] = z.x;
        localToParentMatrix[2][1] = z.y;
        localToParentMatrix[2][2] = z.z;
        localToParentMatrix[2][3] = 0.0f;

        localToParentMatrix[3][0] = translation.x;
        localToParentMatrix[3][1] = translation.y;
        localToParentMatrix[3][2] = translation.z;
        localToParentMatrix[3][3] = 1.0f;


        parentToLocalMatrix[0][0] = x.x;
        parentToLocalMatrix[1][0] = x.y;
        parentToLocalMatrix[2][0] = x.z;

        parentToLocalMatrix[0][1] = y.x;
        parentToLocalMatrix[1][1] = y.y;
        parentToLocalMatrix[2][1] = y.z;

        parentToLocalMatrix[0][2] = z.x;
        parentToLocalMatrix[1][2] = z.y;
        parentToLocalMatrix[2][2] = z.z;

        parentToLocalMatrix[3][0] = -translation.x * x.x - translation.y * x.y - translation.z * x.z;
        parentToLocalMatrix[3][1] = -translation.x * y.x - translation.y * y.y - translation.z * y.z;
        parentToLocalMatrix[3][2] = -translation.x * z.x - translation.y * z.y - translation.z * z.z;

        parentToLocalMatrix[0][3] = 0.0f;
        parentToLocalMatrix[1][3] = 0.0f;
        parentToLocalMatrix[2][3] = 0.0f;
        parentToLocalMatrix[3][3] = 1.0f;

        //TODO: Important: update the euler angle values. At this point they are incoherent with the matrices.
        //If we compute the angles first, the computeMatrices() method can be used after.
    }

    void Transform::computeMatrices() {
        glm::mat4 iden(1.0f);

        localToParentMatrix = glm::translate(iden, translation);
        localToParentMatrix = glm::rotate(localToParentMatrix, glm::radians(rotationEuler.z), glm::vec3(0, 0, 1));
        localToParentMatrix = glm::rotate(localToParentMatrix, glm::radians(rotationEuler.x), glm::vec3(1, 0, 0));
        localToParentMatrix = glm::rotate(localToParentMatrix, glm::radians(rotationEuler.y), glm::vec3(0, 1, 0));

        parentToLocalMatrix = glm::rotate(iden, glm::radians(-rotationEuler.y), glm::vec3(0, 1, 0));
        parentToLocalMatrix = glm::rotate(parentToLocalMatrix, glm::radians(-rotationEuler.x), glm::vec3(1, 0, 0));
        parentToLocalMatrix = glm::rotate(parentToLocalMatrix, glm::radians(-rotationEuler.z), glm::vec3(0, 0, 1));
        parentToLocalMatrix = glm::translate(parentToLocalMatrix, -translation);
    }
}