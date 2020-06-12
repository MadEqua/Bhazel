#include "BhazelEdApp.h"


BZ::Application *createApplication() {
    return new BZ::BhazelEdApp();
}

namespace BZ {

BhazelEdApp::BhazelEdApp() {
    Engine::get().getRendererCoordinator().enable3dRenderer(false);
    Engine::get().getRendererCoordinator().forceOffscreenRendering(true);
    pushLayer(new MainLayer());
}

}