#pragma once

#include "Bhazel/Layer.h"


namespace BZ {

    class ImGuiLayer : public Layer
    {
    public:
        ImGuiLayer();

        void onGraphicsContextCreated() override;
        void onDetach() override;
        void onImGuiRender(TimeDuration deltaTime) override;

        void begin();
        void end();

        float time;
    };
}