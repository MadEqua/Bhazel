#include "bzpch.h"

#include "PipelineState.h"
#include "Bhazel/Renderer/Renderer.h"
#include "Bhazel/Renderer/Framebuffer.h"

//#include "Bhazel/Platform/OpenGL/OpenGLPipelineState.h"
//#include "Bhazel/Platform/D3D11/D3D11PipelineState.h"
#include "Bhazel/Platform/Vulkan/VulkanPipelineState.h"


namespace BZ {

    Ref<PipelineState> PipelineState::create(PipelineStateData& data) {
        switch(Renderer::api) {
            /*case Renderer::API::OpenGL:
                return MakeRef<OpenGLPipelineState>(data);
            case Renderer::API::D3D11:
                return MakeRef<D3D11PipelineState>(data);*/
        case Renderer::API::Vulkan:
            return MakeRef<VulkanPipelineState>(data);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }

    PipelineState::PipelineState(const PipelineStateData &data) :
        data(data) {
        BZ_ASSERT_CORE(data.shader, "PipelineState needs a shader!");
        BZ_ASSERT_CORE(!data.viewports.empty(), "PipelineState needs at least one viewport!");
        BZ_ASSERT_CORE(data.framebuffer, "PipelineState needs a Framebuffer (to get the RenderPass)!");
        BZ_ASSERT_CORE(data.framebuffer->getColorAttachmentCount() == data.blendingState.attachmentBlendingStates.size(), 
            "The number of color attachments defined on the RenderPass must match the number of BlendingStates on PipelineState!");
    }
}
