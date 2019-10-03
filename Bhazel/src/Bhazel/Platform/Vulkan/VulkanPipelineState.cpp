#include "bzpch.h"

#include "VulkanPipelineState.h"

#include "Bhazel/Platform/Vulkan/VulkanShader.h"
#include "Bhazel/Renderer/Buffer.h"

#include "Bhazel/Platform/Vulkan/VulkanFramebuffer.h"


namespace BZ {

    static VkFormat dataTypeToVk(DataType dataType, DataElements dataElements, bool normalized);
    static VkPrimitiveTopology primitiveTopologyToVk(PrimitiveTopology topology);
    static VkPolygonMode polygonModeToVk(PolygonMode mode);
    static VkCullModeFlags cullModeToVk(CullMode mode);
    static VkSampleCountFlagBits sampleCountToVk(uint32 count);
    static VkColorComponentFlags colorMaskToVk(uint32 mask);
    static VkBlendFactor blendingFactorToVk(BlendingFactor factor);
    static VkBlendOp blendingOperationToVk(BlendingOperation operation);
    static VkCompareOp testFunctionToVk(TestFunction function);
    static VkStencilOp stencilOperationsToVk(StencilOperation operation);

    VulkanPipelineState::VulkanPipelineState(PipelineStateData &data) :
        PipelineState(data) {

        //Vertex input data format
        std::vector<VkVertexInputBindingDescription> bindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

        uint32 bufferIndex = 0;
        for(const auto& vb : data.vertexBuffers) {
            const DataLayout& layout = vb->getLayout();

            VkVertexInputBindingDescription bindingDescription = {};
            bindingDescription.binding = bufferIndex;
            bindingDescription.stride = layout.getSizeBytes();
            bindingDescription.inputRate = layout.getDataRate() == DataRate::PerInstance ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;
            bindingDescriptions.emplace_back(bindingDescription);

            uint32 elementIndex = 0;
            for(const auto &element : layout) {

                VkVertexInputAttributeDescription attributeDescription = {};
                attributeDescription.binding = bufferIndex;
                attributeDescription.location = elementIndex;
                attributeDescription.format = dataTypeToVk(element.dataType, element.dataElements, element.normalized);
                attributeDescription.offset = element.offsetBytes;

                attributeDescriptions.emplace_back(attributeDescription);
                elementIndex++;
            }

            bufferIndex++;
        }

        VkPipelineVertexInputStateCreateInfo vertexInputInfoState = {};
        vertexInputInfoState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfoState.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
        vertexInputInfoState.pVertexBindingDescriptions = bindingDescriptions.data();
        vertexInputInfoState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfoState.pVertexAttributeDescriptions = attributeDescriptions.data();

        //TODO: index buffers

        //Input assembly stage setup
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
        inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyState.topology = primitiveTopologyToVk(data.primitiveTopology);
        inputAssemblyState.primitiveRestartEnable = VK_FALSE;

        //Viewport and scissor setup. This is ignored if the viewport (or scissor) is declared dynamic.
        std::vector<VkViewport> viewports(data.viewports.size());
        std::vector<VkRect2D> scissors(data.viewports.size());
        uint32 idx = 0;
        for(const auto& vp : data.viewports) {
            VkViewport viewport = {};
            viewport.x = vp.left;
            viewport.y = vp.top;
            viewport.width = vp.width;
            viewport.height = vp.height;
            viewport.minDepth = vp.minDepth;
            viewport.maxDepth = vp.maxDepth;

            //TODO: support scissor
            VkRect2D scissor = {};
            scissor.offset = { 0, 0 };
            scissor.extent = { static_cast<uint32_t>(vp.width), static_cast<uint32_t>(vp.height) };

            viewports[idx] = viewport;
            scissors[idx] = scissor;
            idx++;
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
        rasterizerState.depthClampEnable = data.rasterizerState.enableDepthClamp;
        rasterizerState.rasterizerDiscardEnable = data.rasterizerState.enableRasterizerDiscard;
        rasterizerState.polygonMode = polygonModeToVk(data.rasterizerState.polygonMode);
        rasterizerState.lineWidth = data.rasterizerState.lineWidth;
        rasterizerState.cullMode = cullModeToVk(data.rasterizerState.cullMode);
        rasterizerState.frontFace = data.rasterizerState.frontCounterClockwise ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizerState.depthBiasEnable = data.rasterizerState.enableDepthBias;
        rasterizerState.depthBiasConstantFactor = data.rasterizerState.depthBiasConstantFactor;
        rasterizerState.depthBiasClamp = data.rasterizerState.depthBiasClamp;
        rasterizerState.depthBiasSlopeFactor = data.rasterizerState.depthBiasSlopeFactor;

        //Multisampling setup
        VkPipelineMultisampleStateCreateInfo multisamplingState = {};
        multisamplingState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisamplingState.sampleShadingEnable = data.multiSampleState.enableSampleShading;
        multisamplingState.rasterizationSamples = sampleCountToVk(data.multiSampleState.sampleCount);
        multisamplingState.minSampleShading = data.multiSampleState.minSampleShading;
        multisamplingState.pSampleMask = nullptr; //TODO
        multisamplingState.alphaToCoverageEnable = data.multiSampleState.enableAlphaToCoverage;
        multisamplingState.alphaToOneEnable = data.multiSampleState.enableAlphaToOne;

        //Depth Stencil setup
        VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
        depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilState.depthTestEnable = data.depthStencilState.enableDepthTest;
        depthStencilState.depthWriteEnable = data.depthStencilState.enableDepthWrite;
        depthStencilState.depthCompareOp = testFunctionToVk(data.depthStencilState.depthTestFunction);
        depthStencilState.depthBoundsTestEnable = data.depthStencilState.enableDepthBoundsTest;
        depthStencilState.stencilTestEnable = data.depthStencilState.enableStencilTest;

        VkStencilOpState stencilFrontState = {};
        depthStencilState.front.failOp = stencilOperationsToVk(data.depthStencilState.frontStencilOperation.failOp);
        depthStencilState.front.passOp = stencilOperationsToVk(data.depthStencilState.frontStencilOperation.passOp);
        depthStencilState.front.depthFailOp = stencilOperationsToVk(data.depthStencilState.frontStencilOperation.depthFailOp);
        depthStencilState.front.compareOp = testFunctionToVk(data.depthStencilState.frontStencilOperation.testFunction);
        depthStencilState.front.compareMask = 0; //TODO
        depthStencilState.front.writeMask = 0; //TODO
        depthStencilState.front.reference = 0; //TODO

        VkStencilOpState stencilBackState = {};
        depthStencilState.back.failOp = stencilOperationsToVk(data.depthStencilState.backStencilOperation.failOp);
        depthStencilState.back.passOp = stencilOperationsToVk(data.depthStencilState.backStencilOperation.passOp);
        depthStencilState.back.depthFailOp = stencilOperationsToVk(data.depthStencilState.backStencilOperation.depthFailOp);
        depthStencilState.back.compareOp = testFunctionToVk(data.depthStencilState.backStencilOperation.testFunction);
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
        for(const auto& blendState : data.blendingState.attachmentBlendingStates) {
            VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
            colorBlendAttachment.blendEnable = blendState.enableBlending;
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

        //TODO
        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0; // Optional
        pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

        BZ_ASSERT_VK(vkCreatePipelineLayout(getGraphicsContext().getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayoutHandle));

        //Shader setup. TODO
        VulkanShader& shader = static_cast<VulkanShader&>(*data.shader);
        VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = shader.getNativeHandle().vertexModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = shader.getNativeHandle().fragmentModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStagesCreateInfos[] = { vertShaderStageInfo, fragShaderStageInfo };

        //Create the graphics pipeline
        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStagesCreateInfos;
        pipelineInfo.pVertexInputState = &vertexInputInfoState;
        pipelineInfo.pInputAssemblyState = &inputAssemblyState;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizerState;
        pipelineInfo.pMultisampleState = &multisamplingState;
        pipelineInfo.pDepthStencilState = &depthStencilState;
        pipelineInfo.pColorBlendState = &colorBlendingState;
        pipelineInfo.pDynamicState = nullptr; // TODO
        pipelineInfo.layout = pipelineLayoutHandle;
        pipelineInfo.renderPass = static_cast<VulkanFramebuffer &>(*data.framebuffer).getNativeHandle().renderPassHandle;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional

        BZ_ASSERT_VK(vkCreateGraphicsPipelines(getGraphicsContext().getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &nativeHandle));
    }

    VulkanPipelineState::~VulkanPipelineState() {
        vkDestroyPipeline(getGraphicsContext().getDevice(), nativeHandle, nullptr);
        vkDestroyPipelineLayout(getGraphicsContext().getDevice(), pipelineLayoutHandle, nullptr);
    }

    static VkFormat dataTypeToVk(DataType dataType, DataElements dataElements, bool normalized) {
        switch(dataType) {
        case DataType::Float32: {
            switch(dataElements) {
            case DataElements::Scalar:
                return VK_FORMAT_R32_SFLOAT;
            case DataElements::Vec2:
                return VK_FORMAT_R32G32_SFLOAT;
            case DataElements::Vec3:
                return VK_FORMAT_R32G32B32_SFLOAT;
            case DataElements::Vec4:
                return VK_FORMAT_R32G32B32A32_SFLOAT;
            case DataElements::Mat2:
            case DataElements::Mat3:
            case DataElements::Mat4:
                BZ_ASSERT_ALWAYS_CORE("Vulkan matrix vertex attributes not implemented.");
                return VK_FORMAT_UNDEFINED;
            default:
                BZ_ASSERT_ALWAYS_CORE("Unknown DataElements.");
                return VK_FORMAT_UNDEFINED;
            }
        }
        case DataType::Float16: {
            switch(dataElements) {
            case DataElements::Scalar:
                return VK_FORMAT_R16_SFLOAT;
            case DataElements::Vec2:
                return VK_FORMAT_R16G16_SFLOAT;
            case DataElements::Vec3:
                return VK_FORMAT_R16G16B16_SFLOAT;
            case DataElements::Vec4:
                return VK_FORMAT_R16G16B16A16_SFLOAT;
            case DataElements::Mat2:
            case DataElements::Mat3:
            case DataElements::Mat4:
                BZ_ASSERT_ALWAYS_CORE("Vulkan matrix vertex attributes not implemented.");
                return VK_FORMAT_UNDEFINED;
            default:
                BZ_ASSERT_ALWAYS_CORE("Unknown DataElements.");
                return VK_FORMAT_UNDEFINED;
            }
        }
        case DataType::Int32: {
            switch(dataElements) {
            case DataElements::Scalar:
                return VK_FORMAT_R32_SINT;
            case DataElements::Vec2:
                return VK_FORMAT_R32G32_SINT;
            case DataElements::Vec3:
                return VK_FORMAT_R32G32B32_SINT;
            case DataElements::Vec4:
                return VK_FORMAT_R32G32B32A32_SINT;
            case DataElements::Mat2:
            case DataElements::Mat3:
            case DataElements::Mat4:
                BZ_ASSERT_ALWAYS_CORE("Vulkan matrix vertex attributes not implemented.");
                return VK_FORMAT_UNDEFINED;
            default:
                BZ_ASSERT_ALWAYS_CORE("Unknown DataElements.");
                return VK_FORMAT_UNDEFINED;
            }
        }
        case DataType::Int16: {
            switch(dataElements) {
            case DataElements::Scalar:
                return normalized ? VK_FORMAT_R16_SNORM : VK_FORMAT_R16_SINT;
            case DataElements::Vec2:
                return normalized ? VK_FORMAT_R16G16_SNORM : VK_FORMAT_R16G16_SINT;
            case DataElements::Vec3:
                return normalized ? VK_FORMAT_R16G16B16_SNORM : VK_FORMAT_R16G16B16_SINT;
            case DataElements::Vec4:
                return normalized ? VK_FORMAT_R16G16B16A16_SNORM : VK_FORMAT_R16G16B16A16_SINT;
            case DataElements::Mat2:
            case DataElements::Mat3:
            case DataElements::Mat4:
                BZ_ASSERT_ALWAYS_CORE("Vulkan matrix vertex attributes not implemented.");
                return VK_FORMAT_UNDEFINED;
            default:
                BZ_ASSERT_ALWAYS_CORE("Unknown DataElements.");
                return VK_FORMAT_UNDEFINED;
            }
        }
        case DataType::Int8: {
            switch(dataElements) {
            case DataElements::Scalar:
                return normalized ? VK_FORMAT_R8_SNORM : VK_FORMAT_R8_SINT;
            case DataElements::Vec2:
                return normalized ? VK_FORMAT_R8G8_SNORM : VK_FORMAT_R8G8_SINT;
            case DataElements::Vec3:
                return normalized ? VK_FORMAT_R8G8B8_SNORM : VK_FORMAT_R8G8B8_SINT;
            case DataElements::Vec4:
                return normalized ? VK_FORMAT_R8G8B8A8_SNORM : VK_FORMAT_R8G8B8A8_SINT;
            case DataElements::Mat2:
            case DataElements::Mat3:
            case DataElements::Mat4:
                BZ_ASSERT_ALWAYS_CORE("Vulkan matrix vertex attributes not implemented.");
                return VK_FORMAT_UNDEFINED;
            default:
                BZ_ASSERT_ALWAYS_CORE("Unknown DataElements.");
                return VK_FORMAT_UNDEFINED;
            }
        }
        case DataType::Uint32: {
            switch(dataElements) {
            case DataElements::Scalar:
                return VK_FORMAT_R32_UINT;
            case DataElements::Vec2:
                return VK_FORMAT_R32G32_UINT;
            case DataElements::Vec3:
                return VK_FORMAT_R32G32B32_UINT;
            case DataElements::Vec4:
                return VK_FORMAT_R32G32B32A32_UINT;
            case DataElements::Mat2:
            case DataElements::Mat3:
            case DataElements::Mat4:
                BZ_ASSERT_ALWAYS_CORE("Vulkan matrix vertex attributes not implemented.");
                return VK_FORMAT_UNDEFINED;
            default:
                BZ_ASSERT_ALWAYS_CORE("Unknown DataElements.");
                return VK_FORMAT_UNDEFINED;
            }
        }
        case DataType::Uint16: {
            switch(dataElements) {
            case DataElements::Scalar:
                return normalized ? VK_FORMAT_R16_UNORM : VK_FORMAT_R16_UINT;
            case DataElements::Vec2:
                return normalized ? VK_FORMAT_R16G16_UNORM : VK_FORMAT_R16G16_UINT;
            case DataElements::Vec3:
                return normalized ? VK_FORMAT_R16G16B16_UNORM : VK_FORMAT_R16G16B16_UINT;
            case DataElements::Vec4:
                return normalized ? VK_FORMAT_R16G16B16A16_UNORM : VK_FORMAT_R16G16B16A16_UINT;
            case DataElements::Mat2:
            case DataElements::Mat3:
            case DataElements::Mat4:
                BZ_ASSERT_ALWAYS_CORE("Vulkan matrix vertex attributes not implemented.");
                return VK_FORMAT_UNDEFINED;
            default:
                BZ_ASSERT_ALWAYS_CORE("Unknown DataElements.");
                return VK_FORMAT_UNDEFINED;
            }
        }
        case DataType::Uint8: {
            switch(dataElements) {
            case DataElements::Scalar:
                return normalized ? VK_FORMAT_R8_UNORM : VK_FORMAT_R8_UINT;
            case DataElements::Vec2:
                return normalized ? VK_FORMAT_R8G8_UNORM : VK_FORMAT_R8G8_UINT;
            case DataElements::Vec3:
                return normalized ? VK_FORMAT_R8G8B8_UNORM : VK_FORMAT_R8G8B8_UINT;
            case DataElements::Vec4:
                return normalized ? VK_FORMAT_R8G8B8A8_UNORM : VK_FORMAT_R8G8B8A8_UINT;
            case DataElements::Mat2:
            case DataElements::Mat3:
            case DataElements::Mat4:
                BZ_ASSERT_ALWAYS_CORE("Vulkan matrix vertex attributes not implemented.");
                return VK_FORMAT_UNDEFINED;
            default:
                BZ_ASSERT_ALWAYS_CORE("Unknown DataElements.");
                return VK_FORMAT_UNDEFINED;
            }
        }
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown DataType.");
            return VK_FORMAT_UNDEFINED;
        }
    }

    static VkPrimitiveTopology primitiveTopologyToVk(PrimitiveTopology topology) {
        switch(topology) {
        case PrimitiveTopology::Points:
            return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case PrimitiveTopology::Lines:
            return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case PrimitiveTopology::Triangles:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case PrimitiveTopology::LineStrip:
            return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case PrimitiveTopology::TriangleStrip:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown PrimitiveTopology!");
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        }
    }

    static VkPolygonMode polygonModeToVk(PolygonMode mode) {
        switch(mode) {
        case PolygonMode::Fill:
            return VK_POLYGON_MODE_FILL;
        case PolygonMode::Line:
            return VK_POLYGON_MODE_LINE;
        case PolygonMode::Point:
            return VK_POLYGON_MODE_POINT;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown PolygonMode!");
            return VK_POLYGON_MODE_FILL;
        }
    }

    static VkCullModeFlags cullModeToVk(CullMode mode) {
        switch(mode) {
        case CullMode::None:
            return VK_CULL_MODE_NONE;
        case CullMode::Front:
            return VK_CULL_MODE_FRONT_BIT;
        case CullMode::Back:
            return VK_CULL_MODE_BACK_BIT;
        case CullMode::FrontAndBack:
            return VK_CULL_MODE_FRONT_AND_BACK;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown CullMode!");
            return VK_CULL_MODE_NONE;
        }
    }

    static VkSampleCountFlagBits sampleCountToVk(uint32 count) {
        switch(count) {
        case 1:
            return VK_SAMPLE_COUNT_1_BIT;
        case 2:
            return VK_SAMPLE_COUNT_2_BIT;
        case 4:
            return VK_SAMPLE_COUNT_4_BIT;
        case 8:
            return VK_SAMPLE_COUNT_8_BIT;
        case 16:
            return VK_SAMPLE_COUNT_16_BIT;
        case 32:
            return VK_SAMPLE_COUNT_32_BIT;
        case 64:
            return VK_SAMPLE_COUNT_64_BIT;
        default:
            BZ_ASSERT_ALWAYS_CORE("Sample count must be a multiple of 2 and <= 64!");
            return VK_SAMPLE_COUNT_1_BIT;
        }
    }

    static VkColorComponentFlags colorMaskToVk(uint32 mask) {
        uint32 result = 0;
        if((1 << (ColorMaskFlags::Red - 1)) && mask)
            result |= VK_COLOR_COMPONENT_R_BIT;
        if((1 << (ColorMaskFlags::Green - 1)) && mask)
            result |= VK_COLOR_COMPONENT_G_BIT;
        if((1 << (ColorMaskFlags::Blue - 1)) && mask)
            result |= VK_COLOR_COMPONENT_B_BIT;
        if((1 << (ColorMaskFlags::Alpha - 1)) && mask)
            result |= VK_COLOR_COMPONENT_A_BIT;
        return result;
    }

    static VkBlendFactor blendingFactorToVk(BlendingFactor factor) {
        switch(factor) {
        case BlendingFactor::Zero:
            return VK_BLEND_FACTOR_ZERO;
        case BlendingFactor::One:
            return VK_BLEND_FACTOR_ONE;
        case BlendingFactor::SourceColor:
            return VK_BLEND_FACTOR_SRC_COLOR;
        case BlendingFactor::OneMinusSourceColor:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case BlendingFactor::DestinationColor:
            return VK_BLEND_FACTOR_DST_COLOR;
        case BlendingFactor::OneMinusDestinationColor:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case BlendingFactor::SourceAlpha:
            return VK_BLEND_FACTOR_SRC_ALPHA;
        case BlendingFactor::OneMinusSourceAlpha:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case BlendingFactor::DestinationAlpha:
            return VK_BLEND_FACTOR_DST_ALPHA;
        case BlendingFactor::OneMinusDestinationAlpha:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case BlendingFactor::ConstantColor:
            return VK_BLEND_FACTOR_CONSTANT_COLOR;
        case BlendingFactor::OneMinusConstantColor:
            return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
        case BlendingFactor::ConstantAlpha:
            return VK_BLEND_FACTOR_CONSTANT_ALPHA;
        case BlendingFactor::OneMinusConstantAlpha:
            return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
        case BlendingFactor::AlphaSaturate:
            return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
        case BlendingFactor::Source1Color:
            return VK_BLEND_FACTOR_SRC1_COLOR;
        case BlendingFactor::OneMinusSource1Color:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
        case BlendingFactor::Source1Alpha:
            return VK_BLEND_FACTOR_SRC_ALPHA;
        case BlendingFactor::OneMinusSource1Alpha:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown BlendingFactor!");
            return VK_BLEND_FACTOR_ONE;
        }
    }

    static VkBlendOp blendingOperationToVk(BlendingOperation operation) {
        switch(operation) {
        case BlendingOperation::Add:
            return VK_BLEND_OP_ADD;
        case BlendingOperation::SourceMinusDestination:
            return VK_BLEND_OP_SUBTRACT;
        case BlendingOperation::DestinationMinusSource:
            return VK_BLEND_OP_REVERSE_SUBTRACT;
        case BlendingOperation::Min:
            return VK_BLEND_OP_MIN;
        case BlendingOperation::Max:
            return VK_BLEND_OP_MAX;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown BlendingOperation!");
            return VK_BLEND_OP_ADD;
        }
    }

    static VkCompareOp testFunctionToVk(TestFunction function) {
        switch(function) {
        case TestFunction::Always:
            return VK_COMPARE_OP_ALWAYS;
        case TestFunction::Never:
            return VK_COMPARE_OP_NEVER;
        case TestFunction::Less:
            return VK_COMPARE_OP_LESS;
        case TestFunction::LessOrEqual:
            return VK_COMPARE_OP_LESS_OR_EQUAL;
        case TestFunction::Greater:
            return VK_COMPARE_OP_GREATER;
        case TestFunction::GreaterOrEqual:
            return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case TestFunction::Equal:
            return VK_COMPARE_OP_EQUAL;
        case TestFunction::NotEqual:
            return VK_COMPARE_OP_EQUAL;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown TestFunction!");
            return VK_COMPARE_OP_ALWAYS;
        }
    }

    static VkStencilOp stencilOperationsToVk(StencilOperation operation) {
        switch(operation) {
        case Keep:
            return VK_STENCIL_OP_KEEP;
        case Zero:
            return VK_STENCIL_OP_ZERO;
        case Replace:
            return VK_STENCIL_OP_REPLACE;
        case IncrementAndClamp:
            return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
        case DecrementAndClamp:
            return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
        case IncrementAndWrap:
            return VK_STENCIL_OP_INCREMENT_AND_WRAP;
        case DecrementAndWrap:
            return VK_STENCIL_OP_DECREMENT_AND_WRAP;
        case Invert:
            return VK_STENCIL_OP_INVERT;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown StencilOperation!");
            return VK_STENCIL_OP_KEEP;
        }
    }
}