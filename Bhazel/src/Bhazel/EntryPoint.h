#pragma once

#ifdef BZ_PLATFORM_WINDOWS

int main(int argc, char** argv) {
    
    BZ::Log::init();
   
    BZ_CORE_WARN("Initialized Core Logger");
    int a = 4;
    BZ_INFO("Hello var={0}", a);
    
    auto app = BZ::createApplication();
    app->run();
    delete app;

    return 0;
}

#endif