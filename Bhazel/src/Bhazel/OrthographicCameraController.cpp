#include "bzpch.h"

#include "OrthographicCameraController.h"
#include "Bhazel/Application.h"
#include "Bhazel/Events/ApplicationEvent.h"
#include "Bhazel/Events/MouseEvent.h"
#include "Bhazel/Input.h"
#include "Bhazel/KeyCodes.h"


namespace BZ {

    OrthographicCameraController::OrthographicCameraController(float aspectRatio, float zoom, bool enableRotation) :
        aspectRatio(aspectRatio),
        zoom(zoom),
        enableRotation(enableRotation),
        camera(-aspectRatio * zoom, aspectRatio * zoom, -zoom, zoom) {
    }

    void OrthographicCameraController::onUpdate(const FrameStats &frameStats) {
        auto cameraPosition = camera.getPosition();

        if(Input::isKeyPressed(BZ_KEY_A)) cameraPosition.x -= cameraMoveSpeed * frameStats.lastFrameTime.asSeconds();
        else if(Input::isKeyPressed(BZ_KEY_D)) cameraPosition.x += cameraMoveSpeed * frameStats.lastFrameTime.asSeconds();
        
        if(Input::isKeyPressed(BZ_KEY_W)) cameraPosition.y += cameraMoveSpeed * frameStats.lastFrameTime.asSeconds();
        else if(Input::isKeyPressed(BZ_KEY_S)) cameraPosition.y -= cameraMoveSpeed * frameStats.lastFrameTime.asSeconds();

        camera.setPosition(cameraPosition);

        if(enableRotation) {
            auto cameraRotation = camera.getRotation();

            if(Input::isKeyPressed(BZ_KEY_Q)) cameraRotation -= cameraRotationSpeed * frameStats.lastFrameTime.asSeconds();
            else if(Input::isKeyPressed(BZ_KEY_E)) cameraRotation += cameraRotationSpeed * frameStats.lastFrameTime.asSeconds();
            camera.setRotation(cameraRotation);
        }
    }

    void OrthographicCameraController::onEvent(Event &e) {
        EventDispatcher dispatcher(e);
        dispatcher.dispatch<MouseScrolledEvent>(BZ_BIND_EVENT_FN(OrthographicCameraController::onMouseScrolled));
        dispatcher.dispatch<WindowResizeEvent>(BZ_BIND_EVENT_FN(OrthographicCameraController::onWindowResized));
    }

    bool OrthographicCameraController::onMouseScrolled(MouseScrolledEvent &e) {
        zoom = std::max(zoom - e.getYOffset() * cameraZoomSpeed, 0.01f);
        camera.computeProjectionMatrix(-aspectRatio * zoom, aspectRatio * zoom, -zoom, zoom);
        return false;
    }

    bool OrthographicCameraController::onWindowResized(WindowResizeEvent &e) {
        aspectRatio = static_cast<float>(e.getWidth()) / static_cast<float>(e.getHeight());
        camera.computeProjectionMatrix(-aspectRatio * zoom, aspectRatio * zoom, -zoom, zoom);
        return false;
    }
}