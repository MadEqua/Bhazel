#pragma once

#include "Layer.h"
#include "Renderer/Renderer.h"


namespace BZ {

    class RendererStatsLayer : public Layer {
    public:
        RendererStatsLayer();

        void onImGuiRender(const FrameStats &frameStats) override;

    private:
        RendererStats visibleFrameStats;

        uint64 refreshPeriodNs;
        uint64 timeAcumNs;
    };
}
