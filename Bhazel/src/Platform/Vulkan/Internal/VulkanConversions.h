#pragma once

#include "Platform/Vulkan/Internal/VulkanIncludes.h"

#include "Graphics/Buffer.h"
#include "Graphics/Texture.h"
#include "Graphics/PipelineState.h"
#include "Graphics/Shader.h"
#include "Graphics/Framebuffer.h"
#include "Graphics/DescriptorSet.h"


namespace BZ {

   uint32 bufferTypeToVK(BufferType bufferType);
   VkFormat dataTypeToVk(DataType dataType, DataElements dataElements, bool normalized);
   VkFormat textureFormatToVk(TextureFormat format);
   TextureFormat vkFormatToTextureFormat(VkFormat format);
   VkPrimitiveTopology primitiveTopologyToVk(PrimitiveTopology topology);
   VkPolygonMode polygonModeToVk(PolygonMode mode);
   VkCullModeFlags cullModeToVk(CullMode mode);
   VkSampleCountFlagBits sampleCountToVk(uint8 count);
   VkColorComponentFlags colorMaskToVk(uint8 mask);
   VkBlendFactor blendingFactorToVk(BlendingFactor factor);
   VkBlendOp blendingOperationToVk(BlendingOperation operation);
   VkCompareOp testFunctionToVk(TestFunction function);
   VkStencilOp stencilOperationsToVk(StencilOperation operation);
   VkAttachmentLoadOp loadOperationToVk(LoadOperation operation);
   VkAttachmentStoreOp storeOperationToVk(StoreOperation operation);
   VkImageLayout textureLayoutToVk(TextureLayout layout);
   VkShaderStageFlagBits shaderStageToVk(ShaderStage stage);
   VkShaderStageFlags shaderStageMaskToVk(uint8 mask);
   VkDescriptorType descriptorTypeToVk(DescriptorType type);
   VkMemoryPropertyFlags memoryTypeToRequiredFlagsVk(MemoryType memoryType);
   VkMemoryPropertyFlags memoryTypeToPreferredFlagsVk(MemoryType memoryType);
   VkFilter filterModeToVk(FilterMode mode);
   VkSamplerAddressMode addressModeToVk(AddressMode mode);
}