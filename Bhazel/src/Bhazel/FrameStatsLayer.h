#pragma once

#include "Bhazel/Layer.h"
#include "Bhazel/Application.h"


namespace BZ {

    class Application;

    class FrameStatsLayer : public Layer {
    public:
        FrameStatsLayer(const Application &application);

        void onImGuiRender(TimeDuration deltaTime) override;

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
