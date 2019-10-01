#pragma once

#include "Bhazel/Renderer/PipelineState.h"

#include "Bhazel/Platform/Vulkan/VulkanIncludes.h"


namespace BZ {

    class VulkanContext;

    class VulkanPipelineState : public PipelineState {
    public:
        VulkanPipelineState(PipelineStateData &data);
        virtual ~VulkanPipelineState() override;

        virtual void bind() override;

    private:
        VulkanContext& context;

        VkPipeline pipelineStateHandle;
        VkPipelineLayout pipelineLayoutHandle;
    };
}
