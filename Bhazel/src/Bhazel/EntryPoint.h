#pragma once

int main(int argc, char** argv) {
    BZ::Log::init();
    auto app = BZ::createApplication();
    app->run();
    delete app;
    return 0;
}