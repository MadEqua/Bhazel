#include "ViewportPanel.h"

#include <Bhazel.h>
#include <imgui.h>


namespace BZ {

ViewportPanel::ViewportPanel() {
}

void ViewportPanel::internalRender() {
    ImGui::Begin("Viewport");
    auto descSet = Engine::get().getRendererCoordinator().getOffscreenTextureDescriptorSet();
    ImGui::Image(descSet, { 320, 180 });
    ImGui::End();
}

}