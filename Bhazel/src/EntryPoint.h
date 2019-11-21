#pragma once

int main(int argc, char** argv) {

    BZ_PROFILE_BEGIN_SESSION("Startup", "BhazelProfile-Startup.json");
    BZ::Log::get();
    auto app = BZ::createApplication();
    BZ_PROFILE_END_SESSION();

    BZ_PROFILE_BEGIN_SESSION("Runtime", "BhazelProfile-Runtime.json");
    app->run();
    BZ_PROFILE_END_SESSION();

    BZ_PROFILE_BEGIN_SESSION("Shutdown", "BhazelProfile-Shutdown.json");
    delete app;
    BZ_PROFILE_END_SESSION();

    return 0;
}