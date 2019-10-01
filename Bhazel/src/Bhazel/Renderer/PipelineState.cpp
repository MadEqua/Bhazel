#include "bzpch.h"

#include "PipelineState.h"
#include "Bhazel/Renderer/Renderer.h"

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
}