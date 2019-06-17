#pragma once

#ifdef BZ_PLATFORM_WINDOWS

int main(int argc, char** argv) {
    
    BZ::Log::init();
   
    BZ_LOG_CORE_INFO("Initialized Logger.");
    
    auto app = BZ::createApplication();
    app->run();
    delete app;

    return 0;
}

#endif