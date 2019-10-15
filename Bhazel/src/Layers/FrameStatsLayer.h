#pragma once

#include "Layer.h"
#include "Core/Application.h"


namespace BZ {


    class FrameStatsLayer : public Layer {
    public:
        FrameStatsLayer(const Application &application);

        void onImGuiRender(const FrameStats &frameStats) override;

    private:
        const Application &application;
        FrameStats visibleFrameStats;

        uint32 refreshPeriodMs;
        uint32 timeAcumMs;

        constexpr static int FRAME_HISTORY_SIZE = 100;
        float frameTimeHistory[FRAME_HISTORY_SIZE] = {};
        uint32 frameTimeHistoryIdx;
    };
}
