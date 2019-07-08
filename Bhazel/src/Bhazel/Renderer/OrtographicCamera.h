#pragma once

#include <glm/glm.hpp>


namespace BZ {

    //Meant for 2D rendering.
    class OrtographicCamera
    {
    public:
        OrtographicCamera(float left, float right, float bottom, float top);

        const glm::vec2& getPosition() const { return position; }
        void setPosition(const glm::vec2 &pos) { position = pos; computeViewMatrix(); }
        void setPosition(float x, float y) { position.x = x; position.y = y; computeViewMatrix(); }

        float getRotation() const { return rotation; }
        void setRotation(float rot) { rotation = rot; computeViewMatrix(); }

        const glm::mat4& getProjectionMatrix() const { return projectionMatrix; }
        const glm::mat4& getViewMatrix() const { return viewMatrix; }
        const glm::mat4& getViewProjectionMatrix() const { return viewProjectionMatrix; }

    private:
        glm::mat4 projectionMatrix;
        glm::mat4 viewMatrix;
        glm::mat4 viewProjectionMatrix;
        
        glm::vec2 position;
        float rotation;

        void computeViewMatrix();
    };
}