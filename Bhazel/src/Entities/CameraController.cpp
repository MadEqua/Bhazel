#include "bzpch.h"

#include "CameraController.h"
#include "Core/Application.h"
#include "Events/WindowEvent.h"
#include "Events/MouseEvent.h"
#include "Core/Input.h"
#include "Core/KeyCodes.h"


namespace BZ {

    OrthographicCameraController::OrthographicCameraController() :
        CameraController(OrthographicCamera(), 1.0f),
        enableRotation(false) {
    }

    OrthographicCameraController::OrthographicCameraController(float left, float right, float bottom, float top, float near, float far, bool enableRotation) :
        CameraController(OrthographicCamera(left, right, bottom, top, near, far), 1.0f),
        originalLeft(left), originalRight(right), originalBottom(bottom), originalTop(top), near(near), far(far),
        enableRotation(enableRotation) {
    }

    void OrthographicCameraController::onUpdate(const FrameStats &frameStats) {
        auto cameraPosition = camera.getPosition();
        bool positionChanged = false;

        Input &input = Application::getInstance().getInput();
        if(input.isKeyPressed(BZ_KEY_A)) {
            cameraPosition.x -= cameraMoveSpeed * frameStats.lastFrameTime.asSeconds();
            positionChanged = true;
        }
        else if(input.isKeyPressed(BZ_KEY_D)) {
            cameraPosition.x += cameraMoveSpeed * frameStats.lastFrameTime.asSeconds();
            positionChanged = true;
        }
        
        if(input.isKeyPressed(BZ_KEY_W)) {
            cameraPosition.y += cameraMoveSpeed * frameStats.lastFrameTime.asSeconds();
            positionChanged = true;
        }
        else if(input.isKeyPressed(BZ_KEY_S)) {
            cameraPosition.y -= cameraMoveSpeed * frameStats.lastFrameTime.asSeconds();
            positionChanged = true;
        }

        if(positionChanged) 
            camera.setPosition(cameraPosition);

        if(enableRotation) {
            auto cameraRotation = camera.getRotation();
            bool rotationChanged = false;

            if(input.isKeyPressed(BZ_KEY_Q)) {
                cameraRotation -= cameraRotationSpeed * frameStats.lastFrameTime.asSeconds();
                rotationChanged = true;
            }
            else if(input.isKeyPressed(BZ_KEY_E)) {
                cameraRotation += cameraRotationSpeed * frameStats.lastFrameTime.asSeconds();
                rotationChanged = true;
            }

            if(rotationChanged)
                camera.setRotation(cameraRotation);
        }
    }

    bool OrthographicCameraController::onMouseScrolled(const MouseScrolledEvent &e) {
        zoom = std::max(zoom - e.getYOffset() * cameraZoomSpeed, 0.01f);
        
        float originalWidth = originalRight - originalLeft;
        float originalHeight = originalTop - originalBottom;

        float newWidth = zoom * originalWidth;
        float newHeight = zoom * originalHeight;

        float diffX = newWidth - originalWidth;
        float diffY = newHeight - originalHeight;

        camera.computeProjectionMatrix(originalLeft - diffX * 0.5f, originalRight + diffX * 0.5f,
                                       originalBottom - diffY * 0.5f, originalTop + diffY * 0.5f,
                                       near, far);
        return false;
    }

    bool OrthographicCameraController::onWindowResized(const WindowResizedEvent &e) {
        return false;
    }


    PerspectiveCameraController::PerspectiveCameraController() :
        CameraController(PerspectiveCamera(), 1.0f),
        fovy(50.0f),
        aspectRatio(16.0f / 10.0f) {
    }

    PerspectiveCameraController::PerspectiveCameraController(float fovy, float aspectRatio) :
        CameraController(PerspectiveCamera(fovy * zoom, aspectRatio), 1.0f),
        fovy(fovy),
        aspectRatio(aspectRatio) {
    }

    void PerspectiveCameraController::onUpdate(const FrameStats &frameStats) {
        Input &input = Application::getInstance().getInput();
        auto mousePosition = input.getMousePosition();
        const auto windowSize = Application::getInstance().getWindow().getDimensions();

        if(mousePosition.x >= 0 && mousePosition.x < windowSize.x && mousePosition.y >= 0 && mousePosition.y < windowSize.y) {
            if(lastMousePosition.x != -1 && lastMousePosition.y != -1) {

                auto dif = mousePosition - lastMousePosition;
                lastMousePosition = mousePosition;

                if(input.isMouseButtonPressed(BZ_MOUSE_BUTTON_RIGHT) && (dif.x != 0.0f || dif.y != 0.0f)) {
                    auto cameraRotation = camera.getRotation();
                    cameraRotation.y -= static_cast<float>(dif.x) * 0.2f;
                    cameraRotation.x -= static_cast<float>(dif.y) * 0.2f;
                    camera.setRotation(cameraRotation);
                }
            }
            else
                lastMousePosition = mousePosition;
        }
        else
            lastMousePosition = {-1, -1};


        bool positionChanged = false;
        glm::vec3 localMovement = {};

        if(input.isKeyPressed(BZ_KEY_A)) {
            localMovement += glm::vec3(-1.0f, 0.0f, 0.0f);
            positionChanged = true;
        }
        else if(input.isKeyPressed(BZ_KEY_D)) {
            localMovement += glm::vec3(1.0, 0.0f, 0.0f);
            positionChanged = true;
        }

        if(input.isKeyPressed(BZ_KEY_W)) {
            localMovement += glm::vec3(0.0f, 0.0f, -1.0f);
            positionChanged = true;
        }
        else if(input.isKeyPressed(BZ_KEY_S)) {
            localMovement += glm::vec3(0.0f, 0.0f, 1.0f);
            positionChanged = true;
        }

        if(positionChanged) {
            glm::vec4 worldMovement = glm::transpose(camera.getViewMatrix()) * 
                glm::vec4(localMovement * cameraMoveSpeed * frameStats.lastFrameTime.asSeconds(), 0.0f);
            camera.setPosition(camera.getPosition() + glm::vec3(worldMovement));
        }
    }

    bool PerspectiveCameraController::onMouseScrolled(const MouseScrolledEvent &e) {
        zoom = std::max(zoom - e.getYOffset() * cameraZoomSpeed, 0.01f);
        camera.computeProjectionMatrix(fovy * zoom, aspectRatio);
        return false;
    }

    bool PerspectiveCameraController::onWindowResized(const WindowResizedEvent &e) {
        aspectRatio = static_cast<float>(e.getWidth()) / static_cast<float>(e.getHeight());
        camera.computeProjectionMatrix(fovy * zoom, aspectRatio);
        return false;
    }
}