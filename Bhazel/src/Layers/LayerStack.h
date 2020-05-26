#pragma once


namespace BZ {

    class Layer;
    struct FrameTiming;
    class Event;

    class LayerStack {
    public:
        ~LayerStack();
        
        void pushLayer(Layer *layer);
        void pushOverlay(Layer *overlay);
        void popLayer(Layer *layer);
        void popOverlay(Layer *overlay);

        void clear();

        void onGraphicsContextCreated();
        void onUpdate(const FrameTiming& frameTiming);
        void onImGuiRender(const FrameTiming& frameTiming);
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