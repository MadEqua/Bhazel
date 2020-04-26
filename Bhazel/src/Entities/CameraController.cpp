#include "bzpch.h"

#include "CameraController.h"
#include "Core/Application.h"
#include "Events/WindowEvent.h"
#include "Events/MouseEvent.h"
#include "Core/Input.h"
#include "Core/KeyCodes.h"


namespace BZ {

    CameraController2D::CameraController2D(OrthographicCamera &camera, float cameraMoveSpeed, bool enableRotation, float cameraRotationSpeed) :
        CameraController(camera),
        originalParameters(camera.getParameters()),
        cameraMoveSpeed(cameraMoveSpeed),
        enableRotation(enableRotation),
        cameraRotationSpeed(cameraRotationSpeed) {
    }

    CameraController2D::CameraController2D(OrthographicCamera &camera, float cameraMoveSpeed, bool enableRotation) :
        CameraController(camera),
        originalParameters(camera.getParameters()),
        cameraMoveSpeed(cameraMoveSpeed),
        enableRotation(enableRotation) {
    }

    void CameraController2D::onUpdate(const FrameStats &frameStats) {
        Input &input = Application::getInstance().getInput();

        if (enableRotation) {
            if (input.isKeyPressed(BZ_KEY_Q)) {
                camera->getTransform().roll(cameraRotationSpeed * frameStats.lastFrameTime.asSeconds(), Space::Parent);
            }
            if (input.isKeyPressed(BZ_KEY_E)) {
                camera->getTransform().roll(-cameraRotationSpeed * frameStats.lastFrameTime.asSeconds(), Space::Parent);
            }
        }

        bool positionChanged = false;
        glm::vec3 movementDir = {};
        if (input.isKeyPressed(BZ_KEY_A)) {
            movementDir.x -= 1.0f;
            positionChanged = true;
        }
        if (input.isKeyPressed(BZ_KEY_D)) {
            movementDir.x += 1.0f;
            positionChanged = true;
        }

        if (input.isKeyPressed(BZ_KEY_W)) {
            movementDir.y += 1.0f;
            positionChanged = true;
        }
        if (input.isKeyPressed(BZ_KEY_S)) {
            movementDir.y -= 1.0f;
            positionChanged = true;
        }

        if (positionChanged) {
            glm::vec3 movementScaled = movementDir * cameraMoveSpeed * frameStats.lastFrameTime.asSeconds();

            //TODO (Mistery alert): Space::Parent is behaving as Space::Local and vice-versa.
            //Only in this CameraController, other Controllers and Entities behave as expected, and they all use the Transform class equally.
            camera->getTransform().translate(movementScaled, Space::Parent);
        }
    }

    bool CameraController2D::onMouseScrolled(const MouseScrolledEvent &e) {
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

    bool CameraController2D::onWindowResized(const WindowResizedEvent &e) {
        OrthographicCamera::Parameters params;
        params.left = -static_cast<float>(e.getWidth()) * 0.5f;
        params.right = static_cast<float>(e.getWidth()) * 0.5f;
        params.bottom = -static_cast<float>(e.getHeight()) * 0.5f;
        params.top = static_cast<float>(e.getHeight()) * 0.5f;
        params.near = originalParameters.near;
        params.far = originalParameters.far;
        camera->setParameters(params);
        return false;
    }


    FreeCameraController::FreeCameraController(PerspectiveCamera &camera, float cameraMoveSpeed) :
        CameraController(camera),
        originalParameters(camera.getParameters()),
        cameraMoveSpeed(cameraMoveSpeed) {
    }

    void FreeCameraController::onUpdate(const FrameStats &frameStats) {
        Input &input = Application::getInstance().getInput();
        auto mousePosition = input.getMousePosition();
        const auto windowSize = Application::getInstance().getWindow().getDimensions();

        if(mousePosition.x >= 0 && mousePosition.x < windowSize.x && mousePosition.y >= 0 && mousePosition.y < windowSize.y) {
            if(lastMousePosition.x != -1 && lastMousePosition.y != -1) {
                auto dif = mousePosition - lastMousePosition;
                if(input.isMouseButtonPressed(BZ_MOUSE_BUTTON_RIGHT) && (dif.x != 0.0f || dif.y != 0.0f)) {
                    camera->getTransform().yaw(-static_cast<float>(dif.x) * 0.2f, Space::Parent);
                    camera->getTransform().pitch(static_cast<float>(dif.y) * 0.2f, Space::Local);
                }
            }
            lastMousePosition = mousePosition;
        }
        else
            lastMousePosition = {-1, -1};


        bool positionChanged = false;
        glm::vec3 movementDir = {};

        if(input.isKeyPressed(BZ_KEY_A)) {
            movementDir.x -= 1.0f;
            positionChanged = true;
        }
        if(input.isKeyPressed(BZ_KEY_D)) {
            movementDir.x += 1.0;
            positionChanged = true;
        }

        if(input.isKeyPressed(BZ_KEY_W)) {
            movementDir.z -= 1.0f;
            positionChanged = true;
        }
        if(input.isKeyPressed(BZ_KEY_S)) {
            movementDir.z += 1.0f;
            positionChanged = true;
        }


        if(positionChanged) {
            float mult = 1.0f;
            if (input.isKeyPressed(BZ_KEY_LEFT_SHIFT)) mult = 10.0f;
            if (input.isKeyPressed(BZ_KEY_LEFT_CONTROL)) mult = 0.05f;
            float speed = cameraMoveSpeed * mult;
            glm::vec3 movementScaled = movementDir * speed * frameStats.lastFrameTime.asSeconds();
            camera->getTransform().translate(movementScaled, Space::Local);
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


    RotateCameraController::RotateCameraController(PerspectiveCamera &camera, float cameraMoveSpeed, float cameraRotationAccel) :
        CameraController(camera),
        originalParameters(camera.getParameters()),
        cameraMoveSpeed(cameraMoveSpeed),
        cameraRotationAccel(cameraRotationAccel) {

        const glm::vec3 &position = camera.getTransform().getTranslation();

        camPosCilindrical[0] = glm::sqrt(position.x * position.x + position.z * position.z);
        camPosCilindrical[1] = glm::atan(position.x, position.z);
        camPosCilindrical[2] = position.y;

        camera.getTransform().lookAt(glm::vec3(0.0f), glm::vec3(0, 1, 0));
    }

    void RotateCameraController::onUpdate(const FrameStats &frameStats) {
        Input &input = Application::getInstance().getInput();
        auto mousePosition = input.getMousePosition();
        const auto windowSize = Application::getInstance().getWindow().getDimensions();

        thetaAccel = 0.0f;
        zAccel = 0.0f;

        if (mousePosition.x >= 0 && mousePosition.x < windowSize.x && mousePosition.y >= 0 && mousePosition.y < windowSize.y) {
            if (lastMousePosition.x != -1 && lastMousePosition.y != -1) {
                auto dif = mousePosition - lastMousePosition;
                if (input.isMouseButtonPressed(BZ_MOUSE_BUTTON_RIGHT)) {
                    if (dif.x != 0.0f) {
                        thetaAccel = dif.x * cameraRotationAccel;
                    }
                    if (dif.y != 0.0f) {
                        zAccel = dif.y * cameraMoveSpeed * 0.45f;
                    }
                }
            }
            lastMousePosition = mousePosition;
        }
        else
            lastMousePosition = { -1, -1 };


        bool changes = false;

        //Acceleration
        thetaVelocity += thetaAccel;
        zVelocity += zAccel;

        //Velocity
        if (glm::abs(thetaVelocity) > 0.0f) {
            camPosCilindrical[1] += thetaVelocity * frameStats.lastFrameTime.asSeconds();
            changes = true;
        }

        if (glm::abs(zVelocity) > 0.0f) {
            camPosCilindrical[2] += zVelocity * frameStats.lastFrameTime.asSeconds();
            changes = true;
        }

        //Drag
        thetaVelocity *= glm::pow(0.001f, frameStats.lastFrameTime.asSeconds());
        if (glm::abs(thetaVelocity) < 0.01f) {
            thetaVelocity = 0.0f;
        }

        zVelocity *= glm::pow(0.001f, frameStats.lastFrameTime.asSeconds());
        if (glm::abs(zVelocity) < 0.01f) {
            zVelocity = 0.0f;
        }

        if (changes) {
            recompute();
        }
    }

    bool RotateCameraController::onMouseScrolled(const MouseScrolledEvent &e) {
        camPosCilindrical[0] -= e.getYOffset() * cameraMoveSpeed;
        if (camPosCilindrical[0] < 0.05f) {
            camPosCilindrical[0] = 0.05f;
        }
        recompute();
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

    void RotateCameraController::recompute() {
        glm::vec3 cameraPosition = { camPosCilindrical[0] * glm::sin(camPosCilindrical[1]),
                                     camPosCilindrical[2],
                                     camPosCilindrical[0] * glm::cos(camPosCilindrical[1]) };

        camera->getTransform().setTranslation(cameraPosition, Space::Parent);
        camera->getTransform().lookAt(glm::vec3(0.0f), glm::vec3(0, 1, 0));
    }
}