#pragma once

#include "Bhazel/Layer.h"


namespace BZ {

    class ImGuiLayer : public Layer
    {
    public:
        ImGuiLayer();

        void onGraphicsContextCreated() override;
        void onDetach() override;
        void onImGuiRender() override;

        void begin();
        void end();

        float time;
    };
}