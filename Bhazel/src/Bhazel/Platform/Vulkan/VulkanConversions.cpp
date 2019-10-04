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
        switch(format.format) {
        case TextureFormatEnum::R8:
            return VK_FORMAT_R8_UNORM;
        case TextureFormatEnum::R8_sRGB:
            return VK_FORMAT_R8_SRGB;
        case TextureFormatEnum::R8G8:
            return VK_FORMAT_R8G8_UNORM;
        case TextureFormatEnum::R8G8_sRGB:
            return VK_FORMAT_R8G8_SRGB;
        case TextureFormatEnum::R8G8B8:
            return VK_FORMAT_R8G8B8_UNORM;
        case TextureFormatEnum::R8G8B8_sRGB:
            return VK_FORMAT_R8G8B8_SRGB;
        case TextureFormatEnum::R8G8B8A8:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case TextureFormatEnum::R8G8B8A8_sRGB:
            return VK_FORMAT_R8G8B8A8_SRGB;
        case TextureFormatEnum::B8G8R8A8:
            return VK_FORMAT_B8G8R8A8_UNORM;
        case TextureFormatEnum::B8G8R8A8_sRGB:
            return VK_FORMAT_B8G8R8A8_SRGB;
        case TextureFormatEnum::D16S8:
            return VK_FORMAT_D16_UNORM_S8_UINT;
        case TextureFormatEnum::D24S8:
            return VK_FORMAT_D24_UNORM_S8_UINT;
        case TextureFormatEnum::Undefined:
            return VK_FORMAT_UNDEFINED;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown TextureFormatEnum!");
            return VK_FORMAT_UNDEFINED;
        };
    }

    TextureFormat vkFormatToTextureFormat(VkFormat format) {
        switch(format) {
        case VK_FORMAT_R8_UNORM:
            return TextureFormatEnum::R8;
        case VK_FORMAT_R8_SRGB:
            return TextureFormatEnum::R8_sRGB;
        case VK_FORMAT_R8G8_UNORM:
            return TextureFormatEnum::R8G8;
        case VK_FORMAT_R8G8_SRGB:
            return TextureFormatEnum::R8G8_sRGB;
        case VK_FORMAT_R8G8B8_UNORM:
            return TextureFormatEnum::R8G8B8;
        case VK_FORMAT_R8G8B8_SRGB:
            return TextureFormatEnum::R8G8B8_sRGB;
        case VK_FORMAT_R8G8B8A8_UNORM:
            return  TextureFormatEnum::R8G8B8A8;
        case VK_FORMAT_R8G8B8A8_SRGB:
            return TextureFormatEnum::R8G8B8A8_sRGB;
        case VK_FORMAT_B8G8R8A8_UNORM:
            return TextureFormatEnum::B8G8R8A8;
        case VK_FORMAT_B8G8R8A8_SRGB:
            return TextureFormatEnum::B8G8R8A8_sRGB;
        case VK_FORMAT_D16_UNORM_S8_UINT:
            return TextureFormatEnum::D16S8;
        case VK_FORMAT_D24_UNORM_S8_UINT:
            return TextureFormatEnum::D24S8;
        case VK_FORMAT_UNDEFINED:
            return TextureFormatEnum::Undefined;
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown VkFormat!");
            return TextureFormatEnum::Undefined;
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

    VkSampleCountFlagBits sampleCountToVk(uint32 count) {
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

    VkColorComponentFlags colorMaskToVk(uint32 mask) {
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
}