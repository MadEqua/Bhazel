#pragma once

#undef near
#undef far

namespace BZ {

    //Meant for 2D rendering.
    class OrthographicCamera
    {
    public:
        OrthographicCamera(float left, float right, float bottom, float top, float near = 0.0f, float far = 1.0f);
        
        void computeProjectionMatrix(float left, float right, float bottom, float top, float near = 0.0f, float far = 1.0f);

        const glm::vec2& getPosition() const { return position; }
        void setPosition(const glm::vec2 &pos) { position = pos; computeViewMatrix(); }
        void setPosition(float x, float y) { position.x = x; position.y = y; computeViewMatrix(); }

        float getRotation() const { return rotation; }
        void setRotation(float rot) { rotation = rot; computeViewMatrix(); }

        const glm::mat4& getProjectionMatrix() const { return projectionMatrix; }
        const glm::mat4& getViewMatrix() const { return viewMatrix; }
        const glm::mat4& getViewProjectionMatrix() const { return viewProjectionMatrix; }

    private:
        void computeViewMatrix();

        glm::mat4 projectionMatrix;
        glm::mat4 viewMatrix;
        glm::mat4 viewProjectionMatrix;
        
        glm::vec2 position;
        float rotation;
    };
}