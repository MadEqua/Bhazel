#pragma once

#include "Graphics/PipelineState.h"

#include "Platform/Vulkan/Internal/VulkanIncludes.h"
#include "Platform/Vulkan/Internal/VulkanGpuObject.h"


namespace BZ {

    class VulkanContext;

    struct VulkanPipelineStateHandles {
        VkPipeline pipeline;
        VkPipelineLayout pipelineLayout;
    };

    class VulkanPipelineState : public PipelineState, public VulkanGpuObject<VulkanPipelineStateHandles> {
    public:
        VulkanPipelineState(PipelineStateData &inData);
        virtual ~VulkanPipelineState() override;

    private:
        void init() override;
        void destroy() override;
    };
}
