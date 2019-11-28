#include "bzpch.h"

#include "FrameStatsLayer.h"

#include <imgui.h>


namespace BZ {

    FrameStatsLayer::FrameStatsLayer(const Application &application) :
        Layer("FrameStatsLayer"),
        application(application),
        timeAcumNs(0),
        refreshPeriodNs(250000000),
        frameTimeHistoryIdx(0) {
    }

    void FrameStatsLayer::onImGuiRender(const FrameStats &frameStats) {
        timeAcumNs += frameStats.lastFrameTime.asNanoseconds();
        if(timeAcumNs >= refreshPeriodNs) {
            timeAcumNs = 0;
            visibleFrameStats = application.getFrameStats();

            frameTimeHistory[frameTimeHistoryIdx] = application.getFrameStats().lastFrameTime.asMillisecondsFloat();
            frameTimeHistoryIdx = (frameTimeHistoryIdx + 1) % FRAME_HISTORY_SIZE;
        }

        if(ImGui::Begin("Frame Stats")) {
            ImGui::Text("Last Frame Time: %.3f ms.", visibleFrameStats.lastFrameTime.asMillisecondsFloat());
            ImGui::Text("FPS: %.3f.", 1.0f / visibleFrameStats.lastFrameTime.asSeconds());
            ImGui::Separator();
            ImGui::Text("Avg Frame Time: %.3f ms.", visibleFrameStats.runningTime.asMillisecondsFloat() / static_cast<float>(visibleFrameStats.frameCount));
            ImGui::Text("Avg FPS: %.3f.", static_cast<float>(visibleFrameStats.frameCount) / visibleFrameStats.runningTime.asSeconds());
            ImGui::Separator();
            ImGui::Text("Frame Count: %d.", visibleFrameStats.frameCount);
            ImGui::Text("Running Time: %.3f seconds.", visibleFrameStats.runningTime.asSeconds());
            ImGui::Separator();
            ImGui::NewLine();
            ImGui::PlotLines("Frame Times", frameTimeHistory, FRAME_HISTORY_SIZE, frameTimeHistoryIdx, "ms", 0.0f, 50.0f, ImVec2(0, 80));
            ImGui::SliderInt("Refresh period ns", reinterpret_cast<int*>(&refreshPeriodNs), 0, 1000000000);
        }
        ImGui::End();
    }
}