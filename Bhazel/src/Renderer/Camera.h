#pragma once

#undef near
#undef far

namespace BZ {

    class Camera {
    public:
        virtual ~Camera() = default;

        const glm::mat4& getProjectionMatrix() const { return projectionMatrix; }
        const glm::mat4& getViewMatrix() const { return viewMatrix; }
        const glm::mat4& getViewProjectionMatrix() const { return viewProjectionMatrix; }

        const glm::vec3& getPosition() const { return position; }
        void setPosition(const glm::vec3 &pos) { position = pos; computeViewMatrix(); }
        void setPosition(float x, float y, float z) { position.x = x; position.y = y; position.z = z; computeViewMatrix(); }

    protected:
        virtual void computeViewMatrix() = 0;

        glm::mat4 projectionMatrix;
        glm::mat4 viewMatrix;
        glm::mat4 viewProjectionMatrix;

        glm::vec3 position = {};
    };


    //Meant for 2D rendering.
    class OrthographicCamera : public Camera {
    public:
        OrthographicCamera();
        OrthographicCamera(float left, float right, float bottom, float top, float near = 0.0f, float far = 1.0f);
        
        void computeProjectionMatrix(float left, float right, float bottom, float top, float near = 0.0f, float far = 1.0f);

        float getRotation() const { return rotation; }
        void setRotation(float rot) { rotation = rot; computeViewMatrix(); }
    
    private:
        void computeViewMatrix() override;

        float rotation = 0.0f;
    };


    class PerspectiveCamera : public Camera {
    public:
        PerspectiveCamera();
        PerspectiveCamera(float fovy, float aspectRatio, float near = 0.1f, float far = 100.0f);
        
        void computeProjectionMatrix(float fovy, float aspectRatio, float near = 0.1f, float far = 100.0f);

        const glm::vec3& getRotation() const { return eulerRotation; }
        void setRotation(const glm::vec3 &rot) { eulerRotation = rot; computeViewMatrix(); }

    private:
        void computeViewMatrix() override;

        glm::vec3 eulerRotation = {};
    };
}