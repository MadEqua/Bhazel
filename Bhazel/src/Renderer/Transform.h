#pragma once


namespace BZ {

    enum class Space {
        Local, Parent
    };

    /*
    * Representing a transform relative to a parent space.
    */
    class Transform {
    public:
        Transform() = default;
        explicit Transform(const glm::vec3 &translation);
        Transform(const glm::vec3 &translation, const glm::vec3 &rotationEuler, const glm::vec3 &scale);
        Transform(const glm::vec3 &translation, const glm::quat &orientation, const glm::vec3 &scale);

        const glm::vec3& getTranslation() const { return translation; }
        void setTranslation(const glm::vec3 &tran, Space space) { setTranslation(tran.x, tran.y, tran.z, space); }
        void setTranslation(float x, float y, float z, Space space);
        void translate(const glm::vec3 &tran, Space space) { translate(tran.x, tran.y, tran.z, space); }
        void translate(float x, float y, float z, Space space);

        const glm::quat& getOrientation() const { return orientation; }
        void setOrientation(const glm::quat &quat, Space space);

        glm::vec3 getRotationEuler() const { return glm::degrees(glm::eulerAngles(orientation)); }
        void setRotationEuler(const glm::vec3 &rot, Space space) { setOrientation(glm::quat(glm::radians(rot)), space); }
        void setRotationEuler(float x, float y, float z, Space space) { setOrientation(glm::quat(glm::radians(glm::vec3(x, y, z))), space); }
        void rotate(const glm::quat &quat, Space space);
        void rotate(float angle, const glm::vec3 &axis, Space space) { rotate(glm::angleAxis(glm::radians(angle), axis), space); }
        void yaw(float angle, Space space) { rotate(angle, glm::vec3(0, 1, 0), space); }
        void pitch(float angle, Space space) { rotate(angle, glm::vec3(1, 0, 0), space); }
        void roll(float angle, Space space) { rotate(angle, glm::vec3(0, 0, 1), space); }

        const glm::vec3& getScale() const { return scale; }
        void setScale(const glm::vec3 &sc) { scale = sc; matricesDirty = true; }
        void setScale(float x, float y, float z) { scale.x = x; scale.y = y; scale.z = z; matricesDirty = true; }
        void setScale(float sc) { scale.x = sc; scale.y = sc; scale.z = sc; matricesDirty = true; }

        //Parameters expected in parent space
        void lookAt(const glm::vec3 &point, const glm::vec3 &up);

        const glm::mat4& getLocalToParentMatrix() const;
        const glm::mat4& getParentToLocalMatrix() const;
        const glm::mat3& getNormalMatrix() const;

    private:
        glm::vec3 translation = {};
        glm::quat orientation = {1.0f, 0.0f, 0.0f, 0.0f};
        glm::vec3 scale = { 1.0f, 1.0f, 1.0f };

        mutable glm::mat4 localToParentMatrix;
        mutable glm::mat4 parentToLocalMatrix;

        //Appropriate matrix to transform normals (same as localToParentMatrix if the scale is uniform)
        mutable glm::mat3 localToParentNormalMatrix;

        mutable bool matricesDirty = true;

        void computeMatrices() const;
    };
}