#include "bzpch.h"

#include "VulkanPipelineState.h"

#include "Platform/Vulkan/VulkanShader.h"
#include "Platform/Vulkan/VulkanRenderPass.h"
#include "Platform/Vulkan/VulkanDescriptorSet.h"
#include "Platform/Vulkan/Internal/VulkanConversions.h"


namespace BZ {

    VulkanPipelineState::VulkanPipelineState(PipelineStateData &inData) :
        PipelineState(inData) {
        init();
    }

    VulkanPipelineState::~VulkanPipelineState() {
        destroy();
    }

    void VulkanPipelineState::init() {

        //Vertex input data format
        std::vector<VkVertexInputBindingDescription> bindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

        if (data.dataLayout.getElementCount() > 0) {
            VkVertexInputBindingDescription bindingDescription = {};
            bindingDescription.binding = 0;
            bindingDescription.stride = data.dataLayout.getSizeBytes();
            bindingDescription.inputRate = data.dataLayout.getDataRate() == DataRate::PerInstance ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;
            bindingDescriptions.emplace_back(bindingDescription);

            uint32 elementIndex = 0;
            for (const auto &element : data.dataLayout) {
                VkVertexInputAttributeDescription attributeDescription = {};
                attributeDescription.binding = 0;
                attributeDescription.location = elementIndex;
                attributeDescription.format = dataTypeToVk(element.dataType, element.dataElements, element.normalized);
                attributeDescription.offset = element.getOffsetBytes();

                attributeDescriptions.emplace_back(attributeDescription);
                elementIndex++;
            }
        }

        VkPipelineVertexInputStateCreateInfo vertexInputInfoState = {};
        vertexInputInfoState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfoState.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
        vertexInputInfoState.pVertexBindingDescriptions = bindingDescriptions.data();
        vertexInputInfoState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfoState.pVertexAttributeDescriptions = attributeDescriptions.data();

        //Input assembly stage setup
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
        inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyState.topology = primitiveTopologyToVk(data.primitiveTopology);
        inputAssemblyState.primitiveRestartEnable = VK_FALSE;

        //Viewport and scissor setup. This is ignored if the viewport (or scissor) is declared dynamic.
        std::vector<VkViewport> viewports(data.viewports.size());
        uint32 idx = 0;
        for (const auto &vp : data.viewports) {
            VkViewport viewport = {};
            viewport.x = vp.rect.left;
            viewport.y = vp.rect.height; //Inverting the space (+y -> up)
            viewport.width = vp.rect.width;
            viewport.height = -vp.rect.height; //Inverting the space (+y -> up) 
            viewport.minDepth = vp.minDepth;
            viewport.maxDepth = vp.maxDepth;
            viewports[idx++] = viewport;
        }

        std::vector<VkRect2D> scissors(data.scissorRects.size());
        idx = 0;
        for (const auto &scissorRect : data.scissorRects) {
            VkRect2D scissor = {};
            scissor.offset.x = scissorRect.rect.left;
            scissor.offset.y = scissorRect.rect.top;
            scissor.extent.width = scissorRect.rect.width;
            scissor.extent.height = scissorRect.rect.height;
            scissors[idx++] = scissor;
        }

        VkPipelineViewportStateCreateInfo viewportState = {};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = static_cast<uint32_t>(viewports.size());
        viewportState.pViewports = viewports.data();
        viewportState.scissorCount = static_cast<uint32_t>(scissors.size());
        viewportState.pScissors = scissors.data();

        //Rasterizer setup
        VkPipelineRasterizationStateCreateInfo rasterizerState = {};
        rasterizerState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizerState.depthClampEnable = data.rasterizerState.enableDepthClamp ? VK_TRUE : VK_FALSE;
        rasterizerState.rasterizerDiscardEnable = data.rasterizerState.enableRasterizerDiscard ? VK_TRUE : VK_FALSE;
        rasterizerState.polygonMode = polygonModeToVk(data.rasterizerState.polygonMode);
        rasterizerState.lineWidth = data.rasterizerState.lineWidth;
        rasterizerState.cullMode = cullModeToVk(data.rasterizerState.cullMode);
        rasterizerState.frontFace = data.rasterizerState.frontFaceCounterClockwise?VK_FRONT_FACE_COUNTER_CLOCKWISE:VK_FRONT_FACE_CLOCKWISE;
        rasterizerState.depthBiasEnable = data.rasterizerState.enableDepthBias ? VK_TRUE : VK_FALSE;
        rasterizerState.depthBiasConstantFactor = data.rasterizerState.depthBiasConstantFactor;
        rasterizerState.depthBiasClamp = data.rasterizerState.depthBiasClamp;
        rasterizerState.depthBiasSlopeFactor = data.rasterizerState.depthBiasSlopeFactor;

        //Multisampling setup
        VkPipelineMultisampleStateCreateInfo multisamplingState = {};
        multisamplingState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisamplingState.sampleShadingEnable = data.multiSampleState.enableSampleShading ? VK_TRUE : VK_FALSE;
        multisamplingState.rasterizationSamples = sampleCountToVk(data.multiSampleState.sampleCount);
        multisamplingState.minSampleShading = data.multiSampleState.minSampleShading;
        multisamplingState.pSampleMask = nullptr; //TODO
        multisamplingState.alphaToCoverageEnable = data.multiSampleState.enableAlphaToCoverage ? VK_TRUE : VK_FALSE;
        multisamplingState.alphaToOneEnable = data.multiSampleState.enableAlphaToOne ? VK_TRUE : VK_FALSE;

        //Depth Stencil setup
        VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
        depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilState.depthTestEnable = data.depthStencilState.enableDepthTest ? VK_TRUE : VK_FALSE;
        depthStencilState.depthWriteEnable = data.depthStencilState.enableDepthWrite ? VK_TRUE : VK_FALSE;
        depthStencilState.depthCompareOp = compareFunctionToVk(data.depthStencilState.depthCompareFunction);
        depthStencilState.depthBoundsTestEnable = data.depthStencilState.enableDepthBoundsTest;
        depthStencilState.stencilTestEnable = data.depthStencilState.enableStencilTest ? VK_TRUE : VK_FALSE;

        VkStencilOpState stencilFrontState = {};
        depthStencilState.front.failOp = stencilOperationsToVk(data.depthStencilState.frontStencilOperation.failOp);
        depthStencilState.front.passOp = stencilOperationsToVk(data.depthStencilState.frontStencilOperation.passOp);
        depthStencilState.front.depthFailOp = stencilOperationsToVk(data.depthStencilState.frontStencilOperation.depthFailOp);
        depthStencilState.front.compareOp = compareFunctionToVk(data.depthStencilState.frontStencilOperation.compareFunction);
        depthStencilState.front.compareMask = 0; //TODO
        depthStencilState.front.writeMask = 0; //TODO
        depthStencilState.front.reference = 0; //TODO

        VkStencilOpState stencilBackState = {};
        depthStencilState.back.failOp = stencilOperationsToVk(data.depthStencilState.backStencilOperation.failOp);
        depthStencilState.back.passOp = stencilOperationsToVk(data.depthStencilState.backStencilOperation.passOp);
        depthStencilState.back.depthFailOp = stencilOperationsToVk(data.depthStencilState.backStencilOperation.depthFailOp);
        depthStencilState.back.compareOp = compareFunctionToVk(data.depthStencilState.backStencilOperation.compareFunction);
        depthStencilState.back.compareMask = 0; //TODO
        depthStencilState.back.writeMask = 0; //TODO
        depthStencilState.back.reference = 0; //TODO

        depthStencilState.front = stencilFrontState;
        depthStencilState.back = stencilBackState;
        depthStencilState.minDepthBounds = data.depthStencilState.minDepthBounds;
        depthStencilState.maxDepthBounds = data.depthStencilState.maxDepthBounds;

        //Blending setup
        std::vector<VkPipelineColorBlendAttachmentState> blendAttachmentStates(data.blendingState.attachmentBlendingStates.size());
        idx = 0;
        for (const auto& blendState : data.blendingState.attachmentBlendingStates) {
            VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
            colorBlendAttachment.blendEnable = blendState.enableBlending ? VK_TRUE : VK_FALSE;
            colorBlendAttachment.srcColorBlendFactor = blendingFactorToVk(blendState.srcColorBlendingFactor);
            colorBlendAttachment.dstColorBlendFactor = blendingFactorToVk(blendState.dstColorBlendingFactor);
            colorBlendAttachment.colorBlendOp = blendingOperationToVk(blendState.colorBlendingOperation);
            colorBlendAttachment.srcAlphaBlendFactor = blendingFactorToVk(blendState.srcAlphaBlendingFactor);
            colorBlendAttachment.dstAlphaBlendFactor = blendingFactorToVk(blendState.srcAlphaBlendingFactor);
            colorBlendAttachment.alphaBlendOp = blendingOperationToVk(blendState.alphaBlendingOperation);
            colorBlendAttachment.colorWriteMask = colorMaskToVk(blendState.writeMask);

            blendAttachmentStates[idx] = colorBlendAttachment;
            idx++;
        }

        VkPipelineColorBlendStateCreateInfo colorBlendingState = {};
        colorBlendingState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendingState.logicOpEnable = VK_FALSE; //TODO logic operations
        colorBlendingState.logicOp = VK_LOGIC_OP_COPY; //TODO
        colorBlendingState.attachmentCount = static_cast<uint32_t>(blendAttachmentStates.size());
        colorBlendingState.pAttachments = blendAttachmentStates.data();
        memcpy(colorBlendingState.blendConstants, &data.blendingState.blendingConstants[0], 4 * sizeof(float));

        //Descriptor Set Layout setup
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts(data.descriptorSetLayouts.size());
        idx = 0;
        for (const auto &layout : data.descriptorSetLayouts) {
            descriptorSetLayouts[idx++] = static_cast<const VulkanDescriptorSetLayout&>(*layout).getNativeHandle();
        }

        std::vector<VkPushConstantRange> pushConstantRanges(data.pushConstantDescs.size());
        idx = 0;
        for (const auto& desc : data.pushConstantDescs) {
            BZ_ASSERT_CORE(desc.size % 4 == 0, "Size must be a multiple of 4!");
            BZ_ASSERT_CORE(desc.offset % 4 == 0, "Offset must be a multiple of 4!");

            pushConstantRanges[idx].offset = desc.offset;
            pushConstantRanges[idx].size = desc.size;
            pushConstantRanges[idx].stageFlags = shaderStageMaskToVk(desc.shaderStageMask);
            idx++;
        }

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32>(pushConstantRanges.size());
        pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();

        BZ_ASSERT_VK(vkCreatePipelineLayout(getDevice(), &pipelineLayoutInfo, nullptr, &nativeHandle.pipelineLayout));

        //Shader setup
        std::array<VkPipelineShaderStageCreateInfo, Shader::SHADER_STAGES_COUNT> shaderStagesCreateInfos;

        uint32 stageCount = 0;
        for (int i = 0; i < Shader::SHADER_STAGES_COUNT; ++i) {
            ShaderStage shaderStage = static_cast<ShaderStage>(i);
            if (data.shader->isStagePresent(shaderStage)) {
                VulkanShader &vulkanShader = static_cast<VulkanShader &>(*data.shader);
                VkPipelineShaderStageCreateInfo shaderCreateInfo = {};
                shaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                shaderCreateInfo.stage = shaderStageToVk(shaderStage);
                shaderCreateInfo.module = vulkanShader.getNativeHandle().modules[i];
                shaderCreateInfo.pName = "main";
                shaderStagesCreateInfos[stageCount++] = shaderCreateInfo;
            }
        }

        //Dynamic State
        std::vector<VkDynamicState> vkDynamicStates(data.dynamicStates.size());
        idx = 0;
        for (auto dynState : data.dynamicStates) {
            vkDynamicStates[idx++] = dynamicStateToVk(dynState);
        }

        VkPipelineDynamicStateCreateInfo dynamicState = {};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(data.dynamicStates.size());
        dynamicState.pDynamicStates = vkDynamicStates.data();

        //Create the graphics pipeline
        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = stageCount;
        pipelineInfo.pStages = shaderStagesCreateInfos.data();
        pipelineInfo.pVertexInputState = &vertexInputInfoState;
        pipelineInfo.pInputAssemblyState = &inputAssemblyState;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizerState;
        pipelineInfo.pMultisampleState = &multisamplingState;
        pipelineInfo.pDepthStencilState = &depthStencilState;
        pipelineInfo.pColorBlendState = &colorBlendingState;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = nativeHandle.pipelineLayout;
        pipelineInfo.renderPass = static_cast<VulkanRenderPass &>(*data.renderPass).getNativeHandle().original;
        pipelineInfo.subpass = data.subPassIndex;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        BZ_ASSERT_VK(vkCreateGraphicsPipelines(getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &nativeHandle.pipeline));
    }

    void VulkanPipelineState::destroy() {
        vkDestroyPipeline(getDevice(), nativeHandle.pipeline, nullptr);
        vkDestroyPipelineLayout(getDevice(), nativeHandle.pipelineLayout, nullptr);
    }
}