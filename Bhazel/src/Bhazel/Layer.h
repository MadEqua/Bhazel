#pragma once

#include "Bhazel/Events/Event.h"
#include "Bhazel/Core/Timer.h"


namespace BZ {

    class Event;

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

        virtual void onUpdate(TimeDuration deltaTime) {}
        virtual void onImGuiRender(TimeDuration deltaTime) {}

        //Receiving events as soon as attached to the LayerStack
        virtual void onEvent(Event &event) {}
        
        inline const std::string &getName() const { return debugName; }
        
    protected:
        std::string debugName;
    };
}