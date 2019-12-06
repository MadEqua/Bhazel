#include "bzpch.h"

#include "Renderer2DStatsLayer.h"
#include "Core/Application.h"

#include <imgui.h>


namespace BZ {

    Renderer2DStatsLayer::Renderer2DStatsLayer() :
        Layer("Renderer2DStatsLayer"),
        timeAcumNs(0),
        refreshPeriodNs(250000000) {
    }

    void Renderer2DStatsLayer::onImGuiRender(const FrameStats &frameStats) {
        timeAcumNs += frameStats.lastFrameTime.asNanoseconds();
        if(timeAcumNs >= refreshPeriodNs) {
            timeAcumNs = 0;
            visibleFrameStats = Renderer2D::getStats();
        }

        if(ImGui::Begin("Renderer2D Stats")) {
            ImGui::Text("Sprite Count: %d", visibleFrameStats.spriteCount);
            ImGui::Text("Draw Call Count: %d", visibleFrameStats.drawCallCount);
            ImGui::Text("Descriptor Set Bind Count: %d", visibleFrameStats.descriptorSetBindCount);
            //ImGui::Text("Tint Push Count: %d", visibleFrameStats.tintPushCount);
            ImGui::Separator();
            ImGui::NewLine();
            ImGui::SliderInt("Refresh period ns", reinterpret_cast<int*>(&refreshPeriodNs), 0, 1000000000);
        }
        ImGui::End();
    }
}