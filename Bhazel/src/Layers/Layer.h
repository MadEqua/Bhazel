#pragma once


namespace BZ {

class Event;
struct FrameTiming;

class Layer {
  public:
    Layer(const char *name = "Layer") : debugName(name) {}
    virtual ~Layer() = default;

    // On Application attach to the Engine.
    // It's safe to create graphics objects.
    virtual void onAttachToEngine() {}
    virtual void onDetach() {}

    virtual void onUpdate(const FrameTiming &frameTiming) {}
    virtual void onImGuiRender(const FrameTiming &frameTiming) {}

    // Receiving events as soon as attached to the LayerStack
    virtual void onEvent(Event &event) {}

    inline const std::string &getName() const { return debugName; }

  protected:
    std::string debugName;
};
}