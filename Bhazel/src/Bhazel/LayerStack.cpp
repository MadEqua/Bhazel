#include "bzpch.h"

#include "LayerStack.h"
#include "Bhazel/Layer.h"
#include "Bhazel/Renderer/Renderer.h"
#include "Bhazel/Events/Event.h"


namespace BZ {

    LayerStack::~LayerStack() {
        for(Layer *layer : layers) {
            layer->onDetach();
            delete layer;
        }
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
        if(it != layers.end()) {
            layers.erase(it);
            layerInsertIndex--;
            layer->onDetach();
        }
    }
    
    void LayerStack::popOverlay(Layer *overlay) {
        auto it = std::find(layers.begin(), layers.end(), overlay);
        if(it != layers.end()) {
            layers.erase(it);
            overlay->onDetach();
        }
    }

    void LayerStack::onGraphicsContextCreated() {
        for(Layer* layer : layers) {
            layer->onGraphicsContextCreated();
        }
    }

    void LayerStack::onUpdate(const FrameStats& frameStats) {
        for(Layer* layer : layers) {
            layer->onUpdate(frameStats);
        }
    }

    void LayerStack::onImGuiRender(const FrameStats& frameStats) {
        for(Layer* layer : layers) {
            layer->onImGuiRender(frameStats);
        }
    }

    void LayerStack::onEvent(Event& event) {
        for(auto it = layers.end(); it != layers.begin(); ) {
            if(event.handled) break;
            (*--it)->onEvent(event);
        }
    }
}