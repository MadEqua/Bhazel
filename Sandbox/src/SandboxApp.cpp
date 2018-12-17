#include <Bhazel.h>

class Sandbox : public BZ::Application {

};

BZ::Application* BZ::createApplication() {
    return new Sandbox();
}