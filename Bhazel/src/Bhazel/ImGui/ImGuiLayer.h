#pragma once

#include "Bhazel/Layer.h"

namespace BZ {

    class MouseButtonPressedEvent;
    class MouseButtonReleasedEvent;
    class MouseMovedEvent;
    class MouseScrolledEvent;
    class KeyPressedEvent;
    class KeyReleasedEvent;
    class KeyTypedEvent;
    class WindowResizeEvent;

    class BZ_API ImGuiLayer : public Layer
    {
    public:
        ImGuiLayer();
        ~ImGuiLayer();

        void onAttach() override;
        void onDetach() override;
        void onUpdate() override;
        void onEvent(Event &event) override;

    private:
        bool onMouseButtonPressedEvent(MouseButtonPressedEvent &e);
        bool onMouseButtonReleasedEvent(MouseButtonReleasedEvent &e);
        bool onMouseMovedEvent(MouseMovedEvent &e);
        bool onMouseScrolledEvent(MouseScrolledEvent &e);
        
        bool onKeyPressedEvent(KeyPressedEvent &e);
        bool onKeyReleasedEvent(KeyReleasedEvent &e);
        bool onKeyTypedEvent(KeyTypedEvent &e);
        
        bool onWindowResizedEvent(WindowResizeEvent &e);

        float time;
    };
}