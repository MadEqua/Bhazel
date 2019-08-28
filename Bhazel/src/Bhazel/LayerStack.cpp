#include "bzpch.h"
#include "LayerStack.h"
#include "Bhazel/Layer.h"

namespace BZ {

    LayerStack::LayerStack() {
    }
    
    LayerStack::~LayerStack() {
        for (Layer *layer : layers) {
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
        if (it != layers.end()) {
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
        for(Layer *layer : layers) {
            layer->onGraphicsContextCreated();
        }
    }

}