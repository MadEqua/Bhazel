#include "MainLayer.h"

#include <imgui.h>


namespace BZ {

void MainLayer::onAttachToEngine() {
}

void MainLayer::onUpdate(const FrameTiming &frameTiming) {
}

void MainLayer::onEvent(Event &event) {
}

void MainLayer::onImGuiRender(const FrameTiming &frameTiming) {
    guiRoot.render();
}

}