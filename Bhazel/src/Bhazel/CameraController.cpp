#include "bzpch.h"

#include "CameraController.h"
#include "Bhazel/Application.h"
#include "Bhazel/Events/WindowEvent.h"
#include "Bhazel/Events/MouseEvent.h"
#include "Bhazel/Input.h"
#include "Bhazel/KeyCodes.h"


namespace BZ {

    OrthographicCameraController::OrthographicCameraController(float aspectRatio, float zoom, bool enableRotation) :
        CameraController(OrthographicCamera(-aspectRatio * zoom, aspectRatio * zoom, -zoom, zoom), aspectRatio, zoom),
        enableRotation(enableRotation) {
    }

    void OrthographicCameraController::onUpdate(const FrameStats &frameStats) {
        auto cameraPosition = camera.getPosition();
        bool positionChanged = false;

        if(Input::isKeyPressed(BZ_KEY_A)) {
            cameraPosition.x -= cameraMoveSpeed * frameStats.lastFrameTime.asSeconds();
            positionChanged = true;
        }
        else if(Input::isKeyPressed(BZ_KEY_D)) {
            cameraPosition.x += cameraMoveSpeed * frameStats.lastFrameTime.asSeconds();
            positionChanged = true;
        }
        
        if(Input::isKeyPressed(BZ_KEY_W)) {
            cameraPosition.y += cameraMoveSpeed * frameStats.lastFrameTime.asSeconds();
            positionChanged = true;
        }
        else if(Input::isKeyPressed(BZ_KEY_S)) {
            cameraPosition.y -= cameraMoveSpeed * frameStats.lastFrameTime.asSeconds();
            positionChanged = true;
        }

        if(positionChanged) 
            camera.setPosition(cameraPosition);

        if(enableRotation) {
            auto cameraRotation = camera.getRotation();
            bool rotationChanged = false;

            if(Input::isKeyPressed(BZ_KEY_Q)) {
                cameraRotation -= cameraRotationSpeed * frameStats.lastFrameTime.asSeconds();
                rotationChanged = true;
            }
            else if(Input::isKeyPressed(BZ_KEY_E)) {
                cameraRotation += cameraRotationSpeed * frameStats.lastFrameTime.asSeconds();
                rotationChanged = true;
            }

            if(rotationChanged)
                camera.setRotation(cameraRotation);
        }
    }

    bool OrthographicCameraController::onMouseScrolled(MouseScrolledEvent &e) {
        zoom = std::max(zoom - e.getYOffset() * cameraZoomSpeed, 0.01f);
        camera.computeProjectionMatrix(-aspectRatio * zoom, aspectRatio * zoom, -zoom, zoom);
        return false;
    }

    bool OrthographicCameraController::onWindowResized(WindowResizedEvent &e) {
        aspectRatio = static_cast<float>(e.getWidth()) / static_cast<float>(e.getHeight());
        camera.computeProjectionMatrix(-aspectRatio * zoom, aspectRatio * zoom, -zoom, zoom);
        return false;
    }


    PerspectiveCameraController::PerspectiveCameraController(float fovy, float aspectRatio, float zoom) :
        CameraController(PerspectiveCamera(fovy * zoom, aspectRatio), aspectRatio, zoom),
        fovy(fovy) {
    }

    void PerspectiveCameraController::onUpdate(const FrameStats &frameStats) {
        auto mousePosition = Input::getMousePosition();
        if(mousePosition.x >= 0 && mousePosition.x < windowSize.x && mousePosition.y >= 0 && mousePosition.y < windowSize.y) {
            if(lastMousePosition.x != -1 && lastMousePosition.y != -1) {

                auto dif = mousePosition - lastMousePosition;
                lastMousePosition = mousePosition;

                if(Input::isMouseButtonPressed(BZ_MOUSE_BUTTON_RIGHT) && (dif.x != 0.0f || dif.y != 0.0f)) {
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

        if(Input::isKeyPressed(BZ_KEY_A)) {
            localMovement += glm::vec3(-1.0f, 0.0f, 0.0f);
            positionChanged = true;
        }
        else if(Input::isKeyPressed(BZ_KEY_D)) {
            localMovement += glm::vec3(1.0, 0.0f, 0.0f);
            positionChanged = true;
        }

        if(Input::isKeyPressed(BZ_KEY_W)) {
            localMovement += glm::vec3(0.0f, 0.0f, -1.0f);
            positionChanged = true;
        }
        else if(Input::isKeyPressed(BZ_KEY_S)) {
            localMovement += glm::vec3(0.0f, 0.0f, 1.0f);
            positionChanged = true;
        }

        if(positionChanged) {
            glm::vec4 worldMovement = glm::transpose(camera.getViewMatrix()) * 
                glm::vec4(localMovement * cameraMoveSpeed * frameStats.lastFrameTime.asSeconds(), 0.0f);
            camera.setPosition(camera.getPosition() + glm::vec3(worldMovement));
        }
    }

    bool PerspectiveCameraController::onMouseScrolled(MouseScrolledEvent &e) {
        zoom = std::max(zoom - e.getYOffset() * cameraZoomSpeed, 0.01f);
        camera.computeProjectionMatrix(fovy * zoom, aspectRatio);
        return false;
    }

    bool PerspectiveCameraController::onWindowResized(WindowResizedEvent &e) {
        windowSize.x = e.getWidth();
        windowSize.y = e.getHeight();
        aspectRatio = static_cast<float>(e.getWidth()) / static_cast<float>(e.getHeight());
        camera.computeProjectionMatrix(fovy * zoom, aspectRatio);
        return false;
    }
}