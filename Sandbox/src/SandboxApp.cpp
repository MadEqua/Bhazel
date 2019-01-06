#include <Bhazel.h>

class ExampleLayer : public BZ::Layer {
public:
	ExampleLayer() : Layer("Example") {
	}

	void onUpdate() override {
		BZ_INFO("ExampleLayer::update");
	}

	void onEvent(BZ::Event &event) override {
		BZ_TRACE("{0}", event);
	}
};

class Sandbox : public BZ::Application {
public:
	Sandbox() {
		pushLayer(new ExampleLayer());
	}
};


BZ::Application* BZ::createApplication() {
    return new Sandbox();
}