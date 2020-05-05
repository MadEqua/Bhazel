#include "bzpch.h"

#include "PipelineState.h"
#include "Core/Application.h"
#include "Graphics/GraphicsContext.h"
#include "Graphics/Graphics.h"
#include "Graphics/RenderPass.h"
#include "Graphics/Shader.h"

#include "Platform/Vulkan/VulkanPipelineState.h"


namespace BZ {

    Ref<PipelineState> PipelineState::create(PipelineStateData& data) {
        switch(Graphics::api) {
        case Graphics::API::Vulkan:
            return MakeRef<VulkanPipelineState>(data);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }

    PipelineState::PipelineState(PipelineStateData &inData) :
        data(inData) {

        BZ_ASSERT_CORE(data.shader, "PipelineState needs a shader!");
        BZ_ASSERT_CORE(std::find(data.dynamicStates.begin(), data.dynamicStates.end(), DynamicState::Viewport) != data.dynamicStates.end() ||
            !data.viewports.empty(),
            "PipelineState with no dynamic Viewport, needs at least one Viewport!");

        BZ_ASSERT_CORE(std::find(data.dynamicStates.begin(), data.dynamicStates.end(), DynamicState::Scissor) != data.dynamicStates.end() ||
            std::find(data.dynamicStates.begin(), data.dynamicStates.end(), DynamicState::Viewport) != data.dynamicStates.end() ||
            data.scissorRects.size() == data.viewports.size(),
            "With non-dynamic Scissor and Viewports the number of Viewports must match the number of ScissorsRects!");

        BZ_ASSERT_CORE(data.renderPass, "PipelineState needs a RenderPass!");
        BZ_ASSERT_CORE(data.renderPass->getColorAttachmentCount() == data.blendingState.attachmentBlendingStates.size(),
            "The number of color attachments defined on the RenderPass must match the number of BlendingStates on PipelineState!");

#ifdef BZ_HOT_RELOAD_SHADERS
        Application::getInstance().getFileWatcher().registerPipelineState(*this);
#endif
    }

    void PipelineState::reload() {
        destroy();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        data.shader->reload();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        init();
    }
}
