#pragma once


namespace BZ {

    class Layer;
    struct FrameStats;
    class Event;

    class LayerStack {
    public:
        ~LayerStack();
        
        void pushLayer(Layer *layer);
        void pushOverlay(Layer *overlay);
        void popLayer(Layer *layer);
        void popOverlay(Layer *overlay);

        void onGraphicsContextCreated();
        void onUpdate(const FrameStats& frameStats);
        void onImGuiRender(const FrameStats& frameStats);
        void onEvent(Event& event);
        
        /*std::vector<Layer*>::iterator begin() { return layers.begin(); }
        std::vector<Layer*>::iterator end() { return layers.end(); }
        std::vector<Layer*>::const_iterator begin() const{ return layers.cbegin(); }
        std::vector<Layer*>::const_iterator end() const { return layers.cend(); }*/

    private:
        std::vector<Layer*> layers;
        uint32 layerInsertIndex = 0;
    };
}