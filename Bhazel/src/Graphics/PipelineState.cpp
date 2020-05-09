#include "bzpch.h"

#include "PipelineState.h"

#include "Graphics/Shader.h"
#include "Graphics/RenderPass.h"
#include "Graphics/DescriptorSet.h"


namespace BZ {

    Ref<PipelineState> PipelineState::create(PipelineStateData& data) {
        return MakeRef<PipelineState>(data);
    }

    PipelineState::PipelineState(PipelineStateData &inData) :
        data(inData) {

        BZ_ASSERT_CORE(data.shader, "PipelineState needs a shader!");
        BZ_ASSERT_CORE(std::find(data.dynamicStates.begin(), data.dynamicStates.end(), VK_DYNAMIC_STATE_VIEWPORT) != data.dynamicStates.end() ||
            !data.viewports.empty(),
            "PipelineState with no dynamic Viewport, needs at least one Viewport!");

        BZ_ASSERT_CORE(std::find(data.dynamicStates.begin(), data.dynamicStates.end(), VK_DYNAMIC_STATE_SCISSOR) != data.dynamicStates.end() ||
            std::find(data.dynamicStates.begin(), data.dynamicStates.end(), VK_DYNAMIC_STATE_VIEWPORT) != data.dynamicStates.end() ||
            data.scissorRects.size() == data.viewports.size(),
            "With non-dynamic Scissor and Viewports the number of Viewports must match the number of ScissorsRects!");

        BZ_ASSERT_CORE(data.renderPass, "PipelineState needs a RenderPass!");
        BZ_ASSERT_CORE(data.renderPass->getColorAttachmentCount() == data.blendingState.attachmentBlendingStates.size(),
            "The number of color attachments defined on the RenderPass must match the number of BlendingStates on PipelineState!");

#ifdef BZ_HOT_RELOAD_SHADERS
        Application::get().getFileWatcher().registerPipelineState(*this);
#endif

        init();
    }

    PipelineState::~PipelineState() {
        destroy();
    }

    void PipelineState::reload() {
        destroy();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        data.shader->reload();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        init();
    }

    void PipelineState::init() {
        //Vertex input data format
        std::vector<VkVertexInputBindingDescription> bindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

        if (data.dataLayout.getElementCount() > 0) {
            VkVertexInputBindingDescription bindingDescription = {};
            bindingDescription.binding = 0;
            bindingDescription.stride = data.dataLayout.getSizeBytes();
            bindingDescription.inputRate = data.dataLayout.getVertexInputRate();
            bindingDescriptions.emplace_back(bindingDescription);

            uint32 elementIndex = 0;
            for (const auto &element : data.dataLayout) {
                VkVertexInputAttributeDescription attributeDescription = {};
                attributeDescription.binding = 0;
                attributeDescription.location = elementIndex;
                attributeDescription.format = element.toVkFormat();
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
        inputAssemblyState.topology = data.primitiveTopology;
        inputAssemblyState.primitiveRestartEnable = VK_FALSE;

        //Viewport and scissor setup. This is ignored if the viewport (or scissor) is declared dynamic.
        for(auto &vp : data.viewports) {
            //Inverting the space (+y -> up)
            vp.y = vp.height;
            vp.height = -vp.height;
        }

        VkPipelineViewportStateCreateInfo viewportState = {};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = static_cast<uint32_t>(data.viewports.size());
        viewportState.pViewports = data.viewports.data();
        viewportState.scissorCount = static_cast<uint32_t>(data.scissorRects.size());
        viewportState.pScissors = data.scissorRects.data();

        //Rasterizer setup
        VkPipelineRasterizationStateCreateInfo rasterizerState = {};
        rasterizerState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizerState.depthClampEnable = data.rasterizerState.enableDepthClamp ? VK_TRUE : VK_FALSE;
        rasterizerState.rasterizerDiscardEnable = data.rasterizerState.enableRasterizerDiscard ? VK_TRUE : VK_FALSE;
        rasterizerState.polygonMode = data.rasterizerState.polygonMode;
        rasterizerState.lineWidth = data.rasterizerState.lineWidth;
        rasterizerState.cullMode = data.rasterizerState.cullMode;
        rasterizerState.frontFace = data.rasterizerState.frontFaceCounterClockwise ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
        rasterizerState.depthBiasEnable = data.rasterizerState.enableDepthBias ? VK_TRUE : VK_FALSE;
        rasterizerState.depthBiasConstantFactor = data.rasterizerState.depthBiasConstantFactor;
        rasterizerState.depthBiasClamp = data.rasterizerState.depthBiasClamp;
        rasterizerState.depthBiasSlopeFactor = data.rasterizerState.depthBiasSlopeFactor;

        //Multisampling setup
        VkPipelineMultisampleStateCreateInfo multisamplingState = {};
        multisamplingState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisamplingState.sampleShadingEnable = data.multiSampleState.enableSampleShading ? VK_TRUE : VK_FALSE;
        multisamplingState.rasterizationSamples = data.multiSampleState.sampleCount;
        multisamplingState.minSampleShading = data.multiSampleState.minSampleShading;
        multisamplingState.pSampleMask = nullptr;// data.multiSampleState.sampleMask;
        multisamplingState.alphaToCoverageEnable = data.multiSampleState.enableAlphaToCoverage ? VK_TRUE : VK_FALSE;
        multisamplingState.alphaToOneEnable = data.multiSampleState.enableAlphaToOne ? VK_TRUE : VK_FALSE;

        //Depth Stencil setup
        VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
        depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilState.depthTestEnable = data.depthStencilState.enableDepthTest ? VK_TRUE : VK_FALSE;
        depthStencilState.depthWriteEnable = data.depthStencilState.enableDepthWrite ? VK_TRUE : VK_FALSE;
        depthStencilState.depthCompareOp = data.depthStencilState.depthCompareOp;
        depthStencilState.depthBoundsTestEnable = data.depthStencilState.enableDepthBoundsTest;
        depthStencilState.stencilTestEnable = data.depthStencilState.enableStencilTest ? VK_TRUE : VK_FALSE;

        VkStencilOpState stencilFrontState = {};
        depthStencilState.front.failOp = data.depthStencilState.frontStencilOperation.failOp;
        depthStencilState.front.passOp = data.depthStencilState.frontStencilOperation.passOp;
        depthStencilState.front.depthFailOp = data.depthStencilState.frontStencilOperation.depthFailOp;
        depthStencilState.front.compareOp = data.depthStencilState.frontStencilOperation.compareOp;
        depthStencilState.front.compareMask = 0; //TODO
        depthStencilState.front.writeMask = 0; //TODO
        depthStencilState.front.reference = 0; //TODO

        VkStencilOpState stencilBackState = {};
        depthStencilState.back.failOp = data.depthStencilState.backStencilOperation.failOp;
        depthStencilState.back.passOp = data.depthStencilState.backStencilOperation.passOp;
        depthStencilState.back.depthFailOp = data.depthStencilState.backStencilOperation.depthFailOp;
        depthStencilState.back.compareOp = data.depthStencilState.backStencilOperation.compareOp;
        depthStencilState.back.compareMask = 0; //TODO
        depthStencilState.back.writeMask = 0; //TODO
        depthStencilState.back.reference = 0; //TODO

        depthStencilState.front = stencilFrontState;
        depthStencilState.back = stencilBackState;
        depthStencilState.minDepthBounds = data.depthStencilState.minDepthBounds;
        depthStencilState.maxDepthBounds = data.depthStencilState.maxDepthBounds;

        //Blending setup
        std::vector<VkPipelineColorBlendAttachmentState> blendAttachmentStates(data.blendingState.attachmentBlendingStates.size());
        uint32 idx = 0;
        for (const auto& blendState : data.blendingState.attachmentBlendingStates) {
            VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
            colorBlendAttachment.blendEnable = blendState.enableBlending ? VK_TRUE : VK_FALSE;
            colorBlendAttachment.srcColorBlendFactor = blendState.srcColorBlendingFactor;
            colorBlendAttachment.dstColorBlendFactor = blendState.dstColorBlendingFactor;
            colorBlendAttachment.colorBlendOp = blendState.colorBlendingOperation;
            colorBlendAttachment.srcAlphaBlendFactor = blendState.srcAlphaBlendingFactor;
            colorBlendAttachment.dstAlphaBlendFactor = blendState.srcAlphaBlendingFactor;
            colorBlendAttachment.alphaBlendOp = blendState.alphaBlendingOperation;
            colorBlendAttachment.colorWriteMask = blendState.writeMask;

            blendAttachmentStates[idx] = colorBlendAttachment;
            idx++;
        }

        VkPipelineColorBlendStateCreateInfo colorBlendingState = {};
        colorBlendingState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendingState.logicOpEnable = VK_FALSE; //TODO logic operations
        colorBlendingState.logicOp = VK_LOGIC_OP_CLEAR; //TODO
        colorBlendingState.attachmentCount = static_cast<uint32_t>(blendAttachmentStates.size());
        colorBlendingState.pAttachments = blendAttachmentStates.data();
        memcpy(colorBlendingState.blendConstants, &data.blendingState.blendingConstants[0], 4 * sizeof(float));

        //Descriptor Set Layout setup
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts(data.descriptorSetLayouts.size());
        idx = 0;
        for (const auto &layout : data.descriptorSetLayouts) {
            descriptorSetLayouts[idx++] = layout->getHandle();
        }

        for (const auto& desc : data.pushConstants) {
            BZ_ASSERT_CORE(desc.size % 4 == 0, "Size must be a multiple of 4!");
            BZ_ASSERT_CORE(desc.offset % 4 == 0, "Offset must be a multiple of 4!");
        }

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32>(data.pushConstants.size());
        pipelineLayoutInfo.pPushConstantRanges = data.pushConstants.data();

        BZ_ASSERT_VK(vkCreatePipelineLayout(getVkDevice(), &pipelineLayoutInfo, nullptr, &handle.pipelineLayout));

        //Shader setup
        std::array<VkPipelineShaderStageCreateInfo, MAX_SHADER_STAGE_COUNT> shaderStagesCreateInfos;

        for (uint32 i = 0; i < data.shader->getStageCount(); ++i) {
            VkPipelineShaderStageCreateInfo shaderCreateInfo = {};
            shaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderCreateInfo.stage = data.shader->getStageData(i).stageFlag;
            shaderCreateInfo.module = data.shader->getHandle().modules[i];
            shaderCreateInfo.pName = "main";
            shaderStagesCreateInfos[i] = shaderCreateInfo;
        }

        //Dynamic State
        VkPipelineDynamicStateCreateInfo dynamicState = {};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(data.dynamicStates.size());
        dynamicState.pDynamicStates = data.dynamicStates.data();

        //Create the graphics pipeline
        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = data.shader->getStageCount();
        pipelineInfo.pStages = shaderStagesCreateInfos.data();
        pipelineInfo.pVertexInputState = &vertexInputInfoState;
        pipelineInfo.pInputAssemblyState = &inputAssemblyState;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizerState;
        pipelineInfo.pMultisampleState = &multisamplingState;
        pipelineInfo.pDepthStencilState = &depthStencilState;
        pipelineInfo.pColorBlendState = &colorBlendingState;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = handle.pipelineLayout;
        pipelineInfo.renderPass = data.renderPass->getHandle().original;
        pipelineInfo.subpass = data.subPassIndex;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        BZ_ASSERT_VK(vkCreateGraphicsPipelines(getVkDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &handle.pipeline));
    }

    void PipelineState::destroy() {
        vkDestroyPipeline(getVkDevice(), handle.pipeline, nullptr);
        vkDestroyPipelineLayout(getVkDevice(), handle.pipelineLayout, nullptr);
    }
}
