#include "bzpch.h"

#include "CameraController.h"
#include "Core/Application.h"
#include "Events/WindowEvent.h"
#include "Events/MouseEvent.h"
#include "Core/Input.h"
#include "Core/KeyCodes.h"


namespace BZ {

    OrthographicCameraController::OrthographicCameraController() {
    }

    OrthographicCameraController::OrthographicCameraController(OrthographicCamera &camera, bool enableRotation) :
        CameraController(camera),
        originalParameters(camera.getParameters()),
        enableRotation(false) {
    }

    /*OrthographicCameraController::OrthographicCameraController(float left, float right, float bottom, float top, float near, float far, bool enableRotation) :
        CameraController(OrthographicCamera(left, right, bottom, top, near, far)),
        originalLeft(left), originalRight(right), originalBottom(bottom), originalTop(top), near(near), far(far),
        enableRotation(enableRotation) {
    }*/

    void OrthographicCameraController::onUpdate(const FrameStats &frameStats) {
        auto cameraPosition = camera->getTransform().getTranslation();
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
            camera->getTransform().setTranslation(cameraPosition);

        if(enableRotation) {
            auto cameraRotation = camera->getRotation();
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
                camera->setRotation(cameraRotation);
        }
    }

    bool OrthographicCameraController::onMouseScrolled(const MouseScrolledEvent &e) {
        zoom = std::max(zoom - e.getYOffset() * cameraZoomSpeed, 0.01f);
        
        float originalWidth = originalParameters.right - originalParameters.left;
        float originalHeight = originalParameters.top - originalParameters.bottom;

        float newWidth = zoom * originalWidth;
        float newHeight = zoom * originalHeight;

        float diffX = newWidth - originalWidth;
        float diffY = newHeight - originalHeight;

        OrthographicCamera::Parameters params;
        params.left = originalParameters.left - diffX * 0.5f;
        params.right = originalParameters.right - diffX * 0.5f;
        params.bottom = originalParameters.bottom - diffY * 0.5f;
        params.top = originalParameters.top - diffY * 0.5f;
        params.near = originalParameters.near;
        params.far = originalParameters.far;
        camera->setParameters(params);

        return false;
    }

    bool OrthographicCameraController::onWindowResized(const WindowResizedEvent &e) {
        return false;
    }


    FreeCameraController::FreeCameraController() {
    }

    FreeCameraController::FreeCameraController(PerspectiveCamera &camera) :
        CameraController(camera),
        originalParameters(camera.getParameters()) {
    }

    /*FreeCameraController::FreeCameraController(float fovy, float aspectRatio) :
        CameraController(PerspectiveCamera(fovy, aspectRatio)),
        fovy(fovy),
        aspectRatio(aspectRatio) {
    }
    */
    void FreeCameraController::onUpdate(const FrameStats &frameStats) {
        Input &input = Application::getInstance().getInput();
        auto mousePosition = input.getMousePosition();
        const auto windowSize = Application::getInstance().getWindow().getDimensions();

        if(mousePosition.x >= 0 && mousePosition.x < windowSize.x && mousePosition.y >= 0 && mousePosition.y < windowSize.y) {
            if(lastMousePosition.x != -1 && lastMousePosition.y != -1) {

                auto dif = mousePosition - lastMousePosition;
                lastMousePosition = mousePosition;

                if(input.isMouseButtonPressed(BZ_MOUSE_BUTTON_RIGHT) && (dif.x != 0.0f || dif.y != 0.0f)) {
                    auto cameraRotation = camera->getTransform().getRotationEuler();
                    cameraRotation.y -= static_cast<float>(dif.x) * 0.2f;
                    cameraRotation.x -= static_cast<float>(dif.y) * 0.2f;
                    camera->getTransform().setRotationEuler(cameraRotation);
                }
            }
            else
                lastMousePosition = mousePosition;
        }
        else
            lastMousePosition = {-1, -1};


        bool positionChanged = false;
        glm::vec3 movementDir = {};

        if(input.isKeyPressed(BZ_KEY_A)) {
            movementDir += glm::vec3(-1.0f, 0.0f, 0.0f);
            positionChanged = true;
        }
        else if(input.isKeyPressed(BZ_KEY_D)) {
            movementDir += glm::vec3(1.0, 0.0f, 0.0f);
            positionChanged = true;
        }

        if(input.isKeyPressed(BZ_KEY_W)) {
            movementDir += glm::vec3(0.0f, 0.0f, -1.0f);
            positionChanged = true;
        }
        else if(input.isKeyPressed(BZ_KEY_S)) {
            movementDir += glm::vec3(0.0f, 0.0f, 1.0f);
            positionChanged = true;
        }

        if(positionChanged) {
            glm::vec3 localMovement = movementDir * cameraMoveSpeed * frameStats.lastFrameTime.asSeconds();
            glm::vec3 worldMovement = glm::transpose(glm::mat3(camera->getViewMatrix())) * localMovement;
            camera->getTransform().setTranslation(camera->getTransform().getTranslation() + worldMovement);
        }
    }

    bool FreeCameraController::onMouseScrolled(const MouseScrolledEvent &e) {
        zoom = std::max(zoom - e.getYOffset() * cameraZoomSpeed, 0.01f);

        PerspectiveCamera::Parameters params;
        params.fovy = originalParameters.fovy * zoom;
        params.aspectRatio = originalParameters.aspectRatio;
        params.near = originalParameters.near;
        params.far = originalParameters.far;
        camera->setParameters(params);
        return false;
    }

    bool FreeCameraController::onWindowResized(const WindowResizedEvent &e) {
        PerspectiveCamera::Parameters params;
        params.fovy = originalParameters.fovy * zoom;
        params.aspectRatio = static_cast<float>(e.getWidth()) / static_cast<float>(e.getHeight());
        params.near = originalParameters.near;
        params.far = originalParameters.far;
        camera->setParameters(params);
        return false;
    }


    RotateCameraController::RotateCameraController() {
    }

    RotateCameraController::RotateCameraController(PerspectiveCamera &camera) :
        CameraController(camera),
        originalParameters(camera.getParameters()) {

        originalDistance = glm::length(camera.getTransform().getTranslation());
        camera.getTransform().lookAt(glm::vec3(0.0f));
    }

    /*RotateCameraController::RotateCameraController(float fovy, float aspectRatio) :
        CameraController(PerspectiveCamera(fovy, aspectRatio)),
        fovy(fovy),
        aspectRatio(aspectRatio) {

        compute();
    }*/

    void RotateCameraController::onUpdate(const FrameStats &frameStats) {
        Input &input = Application::getInstance().getInput();
        auto mousePosition = input.getMousePosition();
        const auto windowSize = Application::getInstance().getWindow().getDimensions();

        /*if (mousePosition.x >= 0 && mousePosition.x < windowSize.x && mousePosition.y >= 0 && mousePosition.y < windowSize.y) {
            if (lastMousePosition.x != -1 && lastMousePosition.y != -1) {

                auto dif = mousePosition - lastMousePosition;
                lastMousePosition = mousePosition;

                if (input.isMouseButtonPressed(BZ_MOUSE_BUTTON_RIGHT) && (dif.x != 0.0f || dif.y != 0.0f)) {
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
            lastMousePosition = { -1, -1 };*/


        bool positionChanged = false;
        glm::vec3 rotationDir = {};

        if (input.isKeyPressed(BZ_KEY_A)) {
            rotationDir += glm::vec3(0.0f, -1.0f, 0.0f);
            positionChanged = true;
        }
        else if (input.isKeyPressed(BZ_KEY_D)) {
            rotationDir += glm::vec3(1.0, 1.0f, 0.0f);
            positionChanged = true;
        }

        if (input.isKeyPressed(BZ_KEY_W)) {
            rotationDir += glm::vec3(0.0f, 0.0f, -1.0f);
            positionChanged = true;
        }
        else if (input.isKeyPressed(BZ_KEY_S)) {
            rotationDir += glm::vec3(0.0f, 0.0f, 1.0f);
            positionChanged = true;
        }

        if (positionChanged) {
            rotationY += rotationDir.y * cameraMoveSpeed * frameStats.lastFrameTime.asSeconds();
            rotationZ += rotationDir.z * cameraMoveSpeed * frameStats.lastFrameTime.asSeconds();
            
            //TODO use rotationZ
            glm::vec3 cameraPosition = { originalDistance * glm::sin(glm::radians(rotationY)), 
                                         camera->getTransform().getTranslation().y,
                                         originalDistance * glm::cos(glm::radians(rotationY)) };

            camera->getTransform().setTranslation(cameraPosition);
            camera->getTransform().lookAt(glm::vec3(0.0f));
        }
    }

    bool RotateCameraController::onMouseScrolled(const MouseScrolledEvent &e) {
        zoom = std::max(zoom - e.getYOffset() * cameraZoomSpeed, 0.01f);

        PerspectiveCamera::Parameters params;
        params.fovy = originalParameters.fovy * zoom;
        params.aspectRatio = originalParameters.aspectRatio;
        params.near = originalParameters.near;
        params.far = originalParameters.far;
        camera->setParameters(params);
        return false;
    }

    bool RotateCameraController::onWindowResized(const WindowResizedEvent &e) {
        PerspectiveCamera::Parameters params;
        params.fovy = originalParameters.fovy * zoom;
        params.aspectRatio = static_cast<float>(e.getWidth()) / static_cast<float>(e.getHeight());
        params.near = originalParameters.near;
        params.far = originalParameters.far;
        camera->setParameters(params);
        return false;
    }

    void RotateCameraController::compute() {
;
    }
}