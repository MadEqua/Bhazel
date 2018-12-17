#pragma once

#include "Core.h"

namespace BZ {

    class BZ_API Application
    {
    public:
        Application();
        ~Application();

        void run();
    };

    //To be defined in Bhazel client applications
    Application* createApplication();
}