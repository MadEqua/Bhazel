#include "Application.h"

#include "Bhazel/Events/ApplicationEvent.h"
#include "Bhazel/Log.h"

namespace BZ {

    Application::Application() {
    }

    Application::~Application() {
    }

    void Application::run() {
        
        WindowResizeEvent e(555, 666);
        BZ_TRACE(e);
        
        while(true);
    }
}