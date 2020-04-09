#include "bzpch.h"

#include "RendererStatsLayer.h"
#include "Core/Application.h"

#include <imgui.h>


namespace BZ {

    RendererStatsLayer::RendererStatsLayer() :
        Layer("RendererStatsLayer"),
        timeAcumNs(0),
        refreshPeriodNs(250000000) {
    }

    void RendererStatsLayer::onImGuiRender(const FrameStats &frameStats) {
        timeAcumNs += frameStats.lastFrameTime.asNanoseconds();
        if(timeAcumNs >= refreshPeriodNs) {
            timeAcumNs = 0;
            visibleFrameStats = Renderer::getStats();
        }

        if(ImGui::Begin("Renderer Stats")) {
            ImGui::Text("Vertex Count: %d", visibleFrameStats.vertexCount);
            ImGui::Text("Triangle Count: %d", visibleFrameStats.triangleCount);
            ImGui::Text("Draw Calls: %d", visibleFrameStats.drawCallCount);
            ImGui::Text("Material Count: %d", visibleFrameStats.materialCount);
            ImGui::Separator();
            ImGui::NewLine();
            ImGui::SliderInt("Refresh period ns", reinterpret_cast<int*>(&refreshPeriodNs), 0, 1000000000);
        }
        ImGui::End();
    }
}