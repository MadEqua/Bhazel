#pragma once

int main(int argc, char** argv) {
    auto app = BZ::createApplication();
    app->run();
    delete app;
    return 0;
}