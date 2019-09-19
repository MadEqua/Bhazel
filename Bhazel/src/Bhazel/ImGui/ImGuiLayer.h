#pragma once

#include "Bhazel/Layer.h"


namespace BZ {

    struct FrameStats;

    class ImGuiLayer : public Layer
    {
    public:
        ImGuiLayer();

        void onGraphicsContextCreated() override;
        void onDetach() override;
        void onImGuiRender(const FrameStats &frameStats) override;

        void begin();
        void end();

        float time;
    };
}