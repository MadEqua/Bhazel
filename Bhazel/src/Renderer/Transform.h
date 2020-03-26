#pragma once


namespace BZ {

    /*
    * Representing a transform relative to a parent space.
    */
    class Transform {
    public:
        Transform();
        explicit Transform(const glm::vec3 &translation);
        Transform(const glm::vec3 &translation, const glm::vec3 &rotationEuler, const glm::vec3 &scale);

        void lookAt(const glm::vec3 &point);

        const glm::vec3& getTranslation() const { return translation; }
        void setTranslation(const glm::vec3 &tran) { translation = tran; computeMatrices(); }
        void setTranslation(float x, float y, float z) { translation.x = x; translation.y = y; translation.z = z; computeMatrices(); }

        const glm::vec3& getRotationEuler() const { return rotationEuler; }
        void setRotationEuler(const glm::vec3 &rot) { rotationEuler = rot; computeMatrices(); }
        void setRotationEuler(float x, float y, float z) { rotationEuler.x = x; rotationEuler.y = y; rotationEuler.z = z; computeMatrices(); }

        const glm::vec3& getScale() const { return scale; }
        void setScale(const glm::vec3 &sc) { scale = sc; computeMatrices(); }
        void setScale(float x, float y, float z) { scale.x = x; scale.y = y; scale.z = z; computeMatrices(); }

        const glm::mat4& getLocalToParentMatrix() const { return localToParentMatrix; }
        const glm::mat4& getParentToLocalMatrix() const { return parentToLocalMatrix; }
        const glm::mat3& getNormalMatrix() const { return normalMatrix; }

    private:
        glm::vec3 translation = {};
        glm::vec3 rotationEuler = {};
        glm::vec3 scale = { 1.0f, 1.0f, 1.0f };

        glm::mat4 localToParentMatrix;
        glm::mat4 parentToLocalMatrix;
        glm::mat3 normalMatrix;

        void computeMatrices();
    };
}