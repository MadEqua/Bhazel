#pragma once

#include "Bhazel/Renderer/PipelineState.h"

#include "Bhazel/Platform/Vulkan/VulkanIncludes.h"
#include "Bhazel/Platform/Vulkan/VulkanGpuObject.h"


namespace BZ {

    class VulkanContext;

    struct VulkanPipelineStateHandles {
        VkPipeline pipeline;
        VkPipelineLayout pipelineLayout;
    };

    class VulkanPipelineState : public PipelineState, public VulkanGpuObject<VulkanPipelineStateHandles> {
    public:
        VulkanPipelineState(PipelineStateData &data);
        virtual ~VulkanPipelineState() override;
    };
}
