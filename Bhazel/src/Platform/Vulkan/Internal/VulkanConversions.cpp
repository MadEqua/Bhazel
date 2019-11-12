#include "bzpch.h"

#include "VulkanConversions.h"


namespace BZ {

    uint32 bufferTypeToVK(BufferType bufferType) {
        switch(bufferType) {
        case BufferType::Vertex:
            return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        case BZ::BufferType::Index:
            return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        case BZ::BufferType::Constant:
            return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown BufferType!");
            return 0;
        }
    }

    VkFormat dataTypeToVk(DataType dataType, DataElements dataElements, bool normalized) {
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

    VkFormat textureFormatToVk(TextureFormat format) {
        switch(format) {
        case TextureFormat::R8:
            return VK_FORMAT_R8_UNORM;
        case TextureFormat::R8_sRGB:
            return VK_FORMAT_R8_SRGB;
        case TextureFormat::R8G8:
            return VK_FORMAT_R8G8_UNORM;
        case TextureFormat::R8G8_sRGB:
            return VK_FORMAT_R8G8_SRGB;
        case TextureFormat::R8G8B8:
            return VK_FORMAT_R8G8B8_UNORM;
        case TextureFormat::R8G8B8_sRGB:
            return VK_FORMAT_R8G8B8_SRGB;
        case TextureFormat::R8G8B8A8:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case TextureFormat::R8G8B8A8_sRGB:
            return VK_FORMAT_R8G8B8A8_SRGB;
        case TextureFormat::B8G8R8A8:
            return VK_FORMAT_B8G8R8A8_UNORM;
        case TextureFormat::B8G8R8A8_sRGB:
            return VK_FORMAT_B8G8R8A8_SRGB;
        case TextureFormat::D16S8:
            return VK_FORMAT_D16_UNORM_S8_UINT;
        case TextureFormat::D24S8:
            return VK_FORMAT_D24_UNORM_S8_UINT;
        case TextureFormat::Undefined:
            return VK_FORMAT_UNDEFINED;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown TextureFormat!");
            return VK_FORMAT_UNDEFINED;
        };
    }

    TextureFormat vkFormatToTextureFormat(VkFormat format) {
        switch(format) {
        case VK_FORMAT_R8_UNORM:
            return TextureFormat::R8;
        case VK_FORMAT_R8_SRGB:
            return TextureFormat::R8_sRGB;
        case VK_FORMAT_R8G8_UNORM:
            return TextureFormat::R8G8;
        case VK_FORMAT_R8G8_SRGB:
            return TextureFormat::R8G8_sRGB;
        case VK_FORMAT_R8G8B8_UNORM:
            return TextureFormat::R8G8B8;
        case VK_FORMAT_R8G8B8_SRGB:
            return TextureFormat::R8G8B8_sRGB;
        case VK_FORMAT_R8G8B8A8_UNORM:
            return  TextureFormat::R8G8B8A8;
        case VK_FORMAT_R8G8B8A8_SRGB:
            return TextureFormat::R8G8B8A8_sRGB;
        case VK_FORMAT_B8G8R8A8_UNORM:
            return TextureFormat::B8G8R8A8;
        case VK_FORMAT_B8G8R8A8_SRGB:
            return TextureFormat::B8G8R8A8_sRGB;
        case VK_FORMAT_D16_UNORM_S8_UINT:
            return TextureFormat::D16S8;
        case VK_FORMAT_D24_UNORM_S8_UINT:
            return TextureFormat::D24S8;
        case VK_FORMAT_UNDEFINED:
            return TextureFormat::Undefined;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown VkFormat!");
            return TextureFormat::Undefined;
        };
    }

    VkPrimitiveTopology primitiveTopologyToVk(PrimitiveTopology topology) {
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

    VkPolygonMode polygonModeToVk(PolygonMode mode) {
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

    VkCullModeFlags cullModeToVk(CullMode mode) {
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

    VkSampleCountFlagBits sampleCountToVk(uint8 count) {
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

    VkColorComponentFlags colorMaskToVk(uint8 mask) {
        uint32 result = 0;
        if(isSet(mask, ColorMaskFlags::Red))
            result |= VK_COLOR_COMPONENT_R_BIT;
        if(isSet(mask, ColorMaskFlags::Green))
            result |= VK_COLOR_COMPONENT_G_BIT;
        if(isSet(mask, ColorMaskFlags::Blue))
            result |= VK_COLOR_COMPONENT_B_BIT;
        if(isSet(mask, ColorMaskFlags::Alpha))
            result |= VK_COLOR_COMPONENT_A_BIT;
        return result;
    }

    VkBlendFactor blendingFactorToVk(BlendingFactor factor) {
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

    VkBlendOp blendingOperationToVk(BlendingOperation operation) {
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

    VkCompareOp testFunctionToVk(TestFunction function) {
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

    VkStencilOp stencilOperationsToVk(StencilOperation operation) {
        switch(operation) {
        case StencilOperation::Keep:
            return VK_STENCIL_OP_KEEP;
        case StencilOperation::Zero:
            return VK_STENCIL_OP_ZERO;
        case StencilOperation::Replace:
            return VK_STENCIL_OP_REPLACE;
        case StencilOperation::IncrementAndClamp:
            return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
        case StencilOperation::DecrementAndClamp:
            return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
        case StencilOperation::IncrementAndWrap:
            return VK_STENCIL_OP_INCREMENT_AND_WRAP;
        case StencilOperation::DecrementAndWrap:
            return VK_STENCIL_OP_DECREMENT_AND_WRAP;
        case StencilOperation::Invert:
            return VK_STENCIL_OP_INVERT;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown StencilOperation!");
            return VK_STENCIL_OP_KEEP;
        }
    }

    VkAttachmentLoadOp loadOperationToVk(LoadOperation operation) {
        switch(operation) {
        case LoadOperation::Load:
            return VK_ATTACHMENT_LOAD_OP_LOAD;
        case LoadOperation::Clear:
            return VK_ATTACHMENT_LOAD_OP_CLEAR;
        case LoadOperation::DontCare:
            return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown LoadOperation!");
            return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        }
    }

    VkAttachmentStoreOp storeOperationToVk(StoreOperation operation) {
        switch(operation) {
        case StoreOperation::Store:
            return VK_ATTACHMENT_STORE_OP_STORE;
        case StoreOperation::DontCare:
            return VK_ATTACHMENT_STORE_OP_DONT_CARE;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown StoreOperation!");
            return VK_ATTACHMENT_STORE_OP_DONT_CARE;
        }
    }

    VkImageLayout textureLayoutToVk(TextureLayout layout) {
        switch(layout) {
        case TextureLayout::Undefined:
            return VK_IMAGE_LAYOUT_UNDEFINED;
        case TextureLayout::General:
            return VK_IMAGE_LAYOUT_GENERAL;
        case TextureLayout::ColorAttachmentOptimal:
            return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case TextureLayout::DepthStencilAttachmentOptimal:
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        case TextureLayout::DepthStencilReadOnlyOptimal:
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        case TextureLayout::ShaderReadOnlyOptimal:
            return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        case TextureLayout::TransferSrcOptimal:
            return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        case TextureLayout::TransferDstOptimal:
            return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        case TextureLayout::Preinitialized:
            return VK_IMAGE_LAYOUT_PREINITIALIZED;
        case TextureLayout::DepthReadOnlyStencilAttachmentOptimal:
            return VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
        case TextureLayout::DepthAttachmentStencilReadOnlyOptimal:
            return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
        case TextureLayout::Present:
            return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        case TextureLayout::SharedPresent:
            return VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown TextureLayout!");
            return VK_IMAGE_LAYOUT_UNDEFINED;
        }
    }

    VkShaderStageFlagBits shaderStageToVk(ShaderStage stage) {
        switch(stage) {
        case ShaderStage::Vertex:
            return VK_SHADER_STAGE_VERTEX_BIT;
        case ShaderStage::TesselationEvaluation:
            return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        case ShaderStage::TesselationControl:
            return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        case ShaderStage::Geometry:
            return VK_SHADER_STAGE_GEOMETRY_BIT;
        case ShaderStage::Fragment:
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        case ShaderStage::Compute:
            return VK_SHADER_STAGE_COMPUTE_BIT;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown ShaderStage!");
            return VK_SHADER_STAGE_VERTEX_BIT;
        }
    }

    VkShaderStageFlags shaderStageMaskToVk(uint8 mask) {
        uint32 result = 0;
        if(isSet(mask, ShaderStageFlags::Vertex))
            result |= VK_SHADER_STAGE_VERTEX_BIT;
        if(isSet(mask, ShaderStageFlags::TesselationControl))
            result |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        if(isSet(mask, ShaderStageFlags::TesselationEvaluation))
            result |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        if(isSet(mask, ShaderStageFlags::Geometry))
            result |= VK_SHADER_STAGE_GEOMETRY_BIT;
        if(isSet(mask, ShaderStageFlags::Fragment))
            result |= VK_SHADER_STAGE_FRAGMENT_BIT;
        if(isSet(mask, ShaderStageFlags::Compute))
            result |= VK_SHADER_STAGE_COMPUTE_BIT;
        return result;
    }

    VkDescriptorType descriptorTypeToVk(DescriptorType type) {
        switch(type) {
        case DescriptorType::Sampler:
            return VK_DESCRIPTOR_TYPE_SAMPLER;
        case DescriptorType::CombinedTextureSampler:
            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        case DescriptorType::SampledTexture:
            return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case DescriptorType::StorageTexture:
            return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        case DescriptorType::ConstantTexelBuffer:
            return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
        case DescriptorType::StorageTexelBuffer:
            return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
        case DescriptorType::ConstantBuffer:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case DescriptorType::StorageBuffer:
            return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case DescriptorType::ConstantBufferDynamic:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        case DescriptorType::StorageBufferDynamic:
            return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        case DescriptorType::InputAttachment:
            return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown DescriptorType!");
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        }
    }

    VkMemoryPropertyFlags memoryTypeToRequiredFlagsVk(MemoryType memoryType) {
        switch(memoryType) {
        case MemoryType::GpuOnly:
            return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        case MemoryType::CpuToGpu:
            return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        case MemoryType::GpuToCpu:
            return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown MemoryType!");
            return 0;
        }
    }

    VkMemoryPropertyFlags memoryTypeToPreferredFlagsVk(MemoryType memoryType) {
        switch(memoryType) {
        case MemoryType::GpuOnly:
            return 0;
        case MemoryType::CpuToGpu:
            return 0;
        case MemoryType::GpuToCpu:
            return VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown MemoryType!");
            return 0;
        }
    }

    VkFilter filterModeToVk(FilterMode mode) {
        switch(mode) {
        case FilterMode::Nearest:
            return VK_FILTER_NEAREST;
        case FilterMode::Linear:
            return VK_FILTER_LINEAR;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown FilterMode!");
            return VK_FILTER_NEAREST;
        }
    }

    VkSamplerAddressMode addressModeToVk(AddressMode mode) {
        switch(mode) {
        case AddressMode::Repeat:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case AddressMode::MirroredRepeat:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case AddressMode::ClampToEdge:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case AddressMode::ClampToBorder:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        case AddressMode::MirrorClampToEdge:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown AddressMode!");
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        }
    }

    VkDynamicState dynamicStateToVk(DynamicState dynamicState) {
        switch(dynamicState) {
        case DynamicState::Scissor:
            return VK_DYNAMIC_STATE_SCISSOR;
        case DynamicState::LineWidth:
            return VK_DYNAMIC_STATE_LINE_WIDTH;
        case DynamicState::DepthBias:
            return VK_DYNAMIC_STATE_DEPTH_BIAS;
        case DynamicState::BlendConstants:
            return VK_DYNAMIC_STATE_BLEND_CONSTANTS;
        case DynamicState::DepthBounds:
            return VK_DYNAMIC_STATE_DEPTH_BOUNDS;
        case DynamicState::StencilCompareMask:
            return VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK;
        case DynamicState::StencilWriteMask:
            return VK_DYNAMIC_STATE_STENCIL_WRITE_MASK;
        case DynamicState::StencilReference:
            return VK_DYNAMIC_STATE_STENCIL_REFERENCE;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown DynamicState!");
            return VK_DYNAMIC_STATE_SCISSOR;
        }
    }
}