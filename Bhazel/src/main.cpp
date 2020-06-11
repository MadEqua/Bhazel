#include "bzpch.h"

#include "Core/Engine.h"


// To be defined in Bhazel client applications.
BZ::Application *createApplication();


int main(int argc, char **argv) {

    BZ_PROFILE_BEGIN_SESSION("Startup", "BhazelProfile-Startup.json");
    BZ::Engine *engine = new BZ::Engine(); // Init Engine.

    BZ::Application *application = createApplication();
    engine->attachApplication(application);
    BZ_PROFILE_END_SESSION();


    BZ_PROFILE_BEGIN_SESSION("Runtime", "BhazelProfile-Runtime.json");
    engine->mainLoop();
    BZ_PROFILE_END_SESSION();


    BZ_PROFILE_BEGIN_SESSION("Shutdown", "BhazelProfile-Shutdown.json");
    delete engine;
    BZ_PROFILE_END_SESSION();

    return 0;
}