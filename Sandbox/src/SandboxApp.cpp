#include <Bhazel.h>

class ExampleLayer : public BZ::Layer {
public:
	ExampleLayer() : Layer("Example") {
	}

	void onUpdate() override {
	}

	void onEvent(BZ::Event &event) override {
	}
};

class Sandbox : public BZ::Application {
public:
	Sandbox() {
		pushLayer(new ExampleLayer());
		pushOverlay(new BZ::ImGuiLayer());
	}
};


BZ::Application* BZ::createApplication() {
    return new Sandbox();
}