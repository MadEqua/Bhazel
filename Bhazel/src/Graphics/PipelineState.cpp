#include "bzpch.h"

#include "PipelineState.h"
#include "Graphics/Graphics.h"
#include "Graphics/Framebuffer.h"

//#include "Platform/OpenGL/OpenGLPipelineState.h"
//#include "Platform/D3D11/D3D11PipelineState.h"
#include "Platform/Vulkan/VulkanPipelineState.h"


namespace BZ {

    Ref<PipelineState> PipelineState::create(PipelineStateData& data) {
        switch(Graphics::api) {
            /*case Graphics::API::OpenGL:
                return MakeRef<OpenGLPipelineState>(data);
            case Graphics::API::D3D11:
                return MakeRef<D3D11PipelineState>(data);*/
        case Graphics::API::Vulkan:
            return MakeRef<VulkanPipelineState>(data);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }

    PipelineState::PipelineState(PipelineStateData &data) :
        data(data) {
        BZ_ASSERT_CORE(data.shader, "PipelineState needs a shader!");
        BZ_ASSERT_CORE(!data.viewports.empty(), "PipelineState needs at least one viewport!");
        BZ_ASSERT_CORE(data.framebuffer, "PipelineState needs a Framebuffer (to get the RenderPass)!");
        BZ_ASSERT_CORE(data.framebuffer->getColorAttachmentCount() == data.blendingState.attachmentBlendingStates.size(), 
            "The number of color attachments defined on the RenderPass must match the number of BlendingStates on PipelineState!");

        //Always add the main descriptor set layout for the engine.
        data.descriptorSetLayouts.insert(data.descriptorSetLayouts.begin(), Graphics::getDescriptorSetLayout());
    }
}
