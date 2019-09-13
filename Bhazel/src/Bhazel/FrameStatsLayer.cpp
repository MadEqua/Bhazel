#include "bzpch.h"

#include "FrameStatsLayer.h"
#include "Application.h"

#include <imgui.h>


namespace BZ {

    FrameStatsLayer::FrameStatsLayer(const Application &application) :
        Layer("FrameStatsLayer"),
        application(application),
        timeAcumMs(0),
        refreshPeriodMs(250),
        frameTimeHistoryIdx(0) {
    }

    void FrameStatsLayer::onImGuiRender(TimeDuration deltaTime) {
        timeAcumMs += deltaTime.asMillisecondsUint();
        if(timeAcumMs >= refreshPeriodMs) {
            timeAcumMs = 0;
            visibleFrameStats = application.getFrameStats();

            frameTimeHistory[frameTimeHistoryIdx] = application.getFrameStats().lastFrameTime.asMillisecondsFloat();
            frameTimeHistoryIdx = (frameTimeHistoryIdx + 1) % FRAME_HISTORY_SIZE;
        }

        if(ImGui::Begin("Stats")) {
            ImGui::Text("Last Frame Time: %.3f ms.", visibleFrameStats.lastFrameTime.asMillisecondsFloat());
            ImGui::Text("Frame Count: %d.", visibleFrameStats.frameCount);
            ImGui::Text("Running Time: %.3f seconds.", visibleFrameStats.runningTime.asSeconds());
            ImGui::Text("Avg Frame Time: %.3f ms.", visibleFrameStats.runningTime.asMillisecondsFloat() / 
                                                    static_cast<float>(visibleFrameStats.frameCount));

            ImGui::PlotLines("Frame Times", frameTimeHistory, FRAME_HISTORY_SIZE, frameTimeHistoryIdx, "ms", 0.0f, 50.0f, ImVec2(0, 80));

            ImGui::Separator();
            ImGui::SliderInt("Refresh period ms", reinterpret_cast<int*>(&refreshPeriodMs), 0, 1000);
        }
        ImGui::End();
    }
}