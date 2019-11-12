#pragma once

#include "Events/Event.h"


namespace BZ {

    class Event;
    struct FrameStats;

    class Layer {
    public:
        Layer(const std::string &name = "Layer");
        virtual ~Layer() = default;
        
        //On attach to the LayerStack
        virtual void onAttach() {}

        //After creation of the GraphicsContext and the Window. 
        //It's safe to create graphics objects.
        virtual void onGraphicsContextCreated() {}

        virtual void onDetach() {}

        virtual void onUpdate(const FrameStats &frameStats) {}
        virtual void onImGuiRender(const FrameStats &frameStats) {}

        //Receiving events as soon as attached to the LayerStack
        virtual void onEvent(Event &event) {}
        
        inline const std::string &getName() const { return debugName; }
        
    protected:
        std::string debugName;
    };
}