#include "bzpch.h"

#include "Transform.h"


namespace BZ {

    Transform::Transform(const glm::vec3 &translation) :
        translation(translation) {
    }

    Transform::Transform(const glm::vec3 &translation, const glm::vec3 &rotationEuler, const glm::vec3 &scale) :
        translation(translation),
        orientation(glm::quat(glm::radians(rotationEuler))),
        scale(scale) {
    }

    Transform::Transform(const glm::vec3 &translation, const glm::quat &orientation, const glm::vec3 &scale) :
        translation(translation),
        orientation(orientation),
        scale(scale) {
    }

    void Transform::setTranslation(float x, float y, float z, Space space) {
        if (space == Space::Parent) {
            translation.x = x;
            translation.y = y;
            translation.z = z;
        }
        else {
            glm::vec4 transLocal(x, y, z, 0.0f);
            glm::vec4 transParent = getLocalToParentMatrix() * transLocal;
            translation.x = transParent.x;
            translation.y = transParent.y;
            translation.z = transParent.z;
        }
        matricesDirty = true;
    }

    void Transform::translate(float x, float y, float z, Space space) {
        if (space == Space::Parent) {
            translation.x += x;
            translation.y += y;
            translation.z += z;
        }
        else {
            glm::vec4 transLocal(x, y, z, 0.0f);
            glm::vec4 transParent = getLocalToParentMatrix() * transLocal;
            translation.x += transParent.x;
            translation.y += transParent.y;
            translation.z += transParent.z;
        }
        matricesDirty = true;
    }

    void Transform::setOrientation(const glm::quat &quat, Space space) {
        if (space == Space::Parent) {
            orientation = quat;
        }
        else {
            glm::vec3 axisParent = getLocalToParentMatrix() * glm::vec4(glm::axis(quat), 0.0f);
            orientation = glm::angleAxis(glm::angle(quat), axisParent);
        }
        matricesDirty = true;
    }

    void Transform::rotate(const glm::quat &quat, Space space) {
        if (space == Space::Parent) {
            orientation = glm::normalize(quat * orientation);
        }
        else {
            glm::vec3 axisParent = getLocalToParentMatrix() * glm::vec4(glm::axis(quat), 0.0f);
            glm::quat orientationParent = glm::angleAxis(glm::angle(quat), axisParent);
            orientation = glm::normalize(orientationParent * orientation);
        }
        matricesDirty = true;
    }

    void Transform::lookAt(const glm::vec3 &point, const glm::vec3 &up) {
        glm::vec3 z = glm::normalize(translation - point);
        glm::vec3 x = glm::normalize(glm::cross(up, z));
        glm::vec3 y = glm::normalize(glm::cross(z, x));

        glm::mat3 rot(x, y, z);
        orientation = glm::quat(rot);
        matricesDirty = true;
    }

    const glm::mat4& Transform::getLocalToParentMatrix() const {
        if (matricesDirty) {
            computeMatrices();
        }
        return localToParentMatrix;
    }

    const glm::mat4& Transform::getParentToLocalMatrix() const {
        if (matricesDirty) {
            computeMatrices();
        }
        return parentToLocalMatrix;
    }

    const glm::mat3& Transform::getNormalMatrix() const {
        if (matricesDirty) {
            computeMatrices();
        }
        return localToParentNormalMatrix;
    }

    void Transform::computeMatrices() const {
        glm::mat4 iden(1.0f);
        glm::mat4 rot(orientation);

        localToParentMatrix = glm::translate(iden, translation);
        localToParentMatrix = localToParentMatrix * rot;
        localToParentMatrix = glm::scale(localToParentMatrix, scale);

        parentToLocalMatrix = glm::scale(iden, 1.0f / scale);
        parentToLocalMatrix = parentToLocalMatrix * glm::transpose(rot);
        parentToLocalMatrix = glm::translate(parentToLocalMatrix, -translation);

        if (scale.x == scale.y && scale.x == scale.z)
            localToParentNormalMatrix = glm::mat3(localToParentMatrix);
        else
            localToParentNormalMatrix = glm::transpose(glm::mat3(localToParentMatrix));

        matricesDirty = false;
    }
}