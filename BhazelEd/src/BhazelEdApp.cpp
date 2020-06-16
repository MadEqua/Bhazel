#include "BhazelEdApp.h"


BZ::Application *createApplication() {
    return new BZ::BhazelEdApp();
}

namespace BZ {

BhazelEdApp::BhazelEdApp() {
    settings.editorMode = true;

    pushLayer(new MainLayer());
}

}