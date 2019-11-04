#include "bzpch.h"

#include "PipelineState.h"
#include "Core/Application.h"
#include "Graphics/GraphicsContext.h"
#include "Graphics/Graphics.h"
#include "Graphics/Framebuffer.h"

//#include "Platform/OpenGL/OpenGLPipelineState.h"
//#include "Platform/D3D11/D3D11PipelineState.h"
#include "Platform/Vulkan/VulkanPipelineState.h"


namespace BZ {

    Ref<PipelineState> PipelineState::create(PipelineStateData& data) {
        switch(Graphics::api) {
            /*case Graphics::API::OpenGL:
                return MakeRef<OpenGLPipelineState>(inData);
            case Graphics::API::D3D11:
                return MakeRef<D3D11PipelineState>(inData);*/
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

        if(!data.framebuffer)
            data.framebuffer = Application::getInstance().getGraphicsContext().getCurrentFrameFramebuffer();

        BZ_ASSERT_CORE(data.framebuffer->getColorAttachmentCount() == data.blendingState.attachmentBlendingStates.size(),
                       "The number of color attachments defined on the RenderPass must match the number of BlendingStates on PipelineState!");


        //Always add the main descriptor set layouts for the engine.
        data.descriptorSetLayouts.insert(data.descriptorSetLayouts.begin(), Graphics::getDescriptorSetLayout());
        data.descriptorSetLayouts.insert(data.descriptorSetLayouts.begin(), Graphics::getDescriptorSetLayout());
    }
}
