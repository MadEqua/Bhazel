#include "bzpch.h"

#include "LayerStack.h"

#include "Events/Event.h"
#include "Layers/Layer.h"


namespace BZ {

LayerStack::~LayerStack() {
    clear();
}

void LayerStack::pushLayer(Layer *layer) {
    layers.emplace(layers.begin() + layerInsertIndex, layer);
    layerInsertIndex++;
    layer->onAttach();
}

void LayerStack::pushOverlay(Layer *overlay) {
    layers.emplace_back(overlay);
    overlay->onAttach();
}

void LayerStack::popLayer(Layer *layer) {
    auto it = std::find(layers.begin(), layers.end(), layer);
    if (it != layers.end()) {
        layers.erase(it);
        layerInsertIndex--;
        layer->onDetach();
    }
}

void LayerStack::popOverlay(Layer *overlay) {
    auto it = std::find(layers.begin(), layers.end(), overlay);
    if (it != layers.end()) {
        layers.erase(it);
        overlay->onDetach();
    }
}

void LayerStack::clear() {
    for (Layer *layer : layers) {
        layer->onDetach();
        delete layer;
    }
    layers.clear();
}

void LayerStack::onGraphicsContextCreated() {
    for (Layer *layer : layers) {
        layer->onGraphicsContextCreated();
    }
}

void LayerStack::onUpdate(const FrameTiming &frameTiming) {
    for (Layer *layer : layers) {
        layer->onUpdate(frameTiming);
    }
}

void LayerStack::onImGuiRender(const FrameTiming &frameTiming) {
    for (Layer *layer : layers) {
        layer->onImGuiRender(frameTiming);
    }
}

void LayerStack::onEvent(Event &event) {
    for (auto it = layers.end(); it != layers.begin();) {
        if (event.handled)
            break;
        (*--it)->onEvent(event);
    }
}
}