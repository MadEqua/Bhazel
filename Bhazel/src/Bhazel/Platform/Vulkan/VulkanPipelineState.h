#pragma once

#include "Bhazel/Renderer/PipelineState.h"

#include "Bhazel/Platform/Vulkan/VulkanIncludes.h"
#include "Bhazel/Platform/Vulkan/VulkanGpuObject.h"


namespace BZ {

    class VulkanContext;

    class VulkanPipelineState : public PipelineState, public VulkanGpuObject<VkPipeline> {
    public:
        VulkanPipelineState(PipelineStateData &data);
        virtual ~VulkanPipelineState() override;

    private:
        VkPipelineLayout pipelineLayoutHandle;
    };
}
