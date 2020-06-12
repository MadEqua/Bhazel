#include "MainLayer.h"

#include <imgui.h>


namespace BZ {

void MainLayer::onAttachToEngine() {
    const glm::vec2 WINDOW_DIMS = Engine::get().getWindow().getDimensionsFloat();
    const glm::vec2 WINDOW_HALF_DIMS = WINDOW_DIMS * 0.5f;

    camera = OrthographicCamera(-WINDOW_HALF_DIMS.x, WINDOW_HALF_DIMS.x, -WINDOW_HALF_DIMS.y, WINDOW_HALF_DIMS.y);
    camera.getTransform().setTranslation(WINDOW_HALF_DIMS.x, WINDOW_HALF_DIMS.y, 0.0f, Space::Parent);
}

void MainLayer::onUpdate(const FrameTiming &frameTiming) {
    Renderer2D::begin(camera);
    Renderer2D::renderQuad({ 200, 0 }, { 10, 10 }, 0.0f, { 0.2f, 0.9f, 0.1f, 1.0f });
    Renderer2D::renderQuad({ 0, 0 }, { 200, 200 }, 0.0f, { 0.2f, 0.9f, 0.1f, 1.0f });
    Renderer2D::end();
}

void MainLayer::onEvent(Event &event) {
}

void MainLayer::onImGuiRender(const FrameTiming &frameTiming) {
    guiRoot.render();
}

}