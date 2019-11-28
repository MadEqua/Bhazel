#pragma once

#include "Layer.h"
#include "Renderer/Renderer2D.h"


namespace BZ {

    class Renderer2DStatsLayer : public Layer {
    public:
        Renderer2DStatsLayer();

        void onImGuiRender(const FrameStats &frameStats) override;

    private:
        Renderer2DStats visibleFrameStats;

        uint64 refreshPeriodNs;
        uint64 timeAcumNs;
    };
}
