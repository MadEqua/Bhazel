#pragma once

#include "Bhazel/Platform/Vulkan/VulkanIncludes.h"

#include "Bhazel/Renderer/Buffer.h"
#include "Bhazel/Renderer/Texture.h"
#include "Bhazel/Renderer/PipelineState.h"
#include "Bhazel/Renderer/Framebuffer.h"


namespace BZ {

   uint32 bufferTypeToVK(BufferType bufferType);
   VkFormat dataTypeToVk(DataType dataType, DataElements dataElements, bool normalized);
   VkFormat textureFormatToVk(TextureFormat format);
   TextureFormat vkFormatToTextureFormat(VkFormat format);
   VkPrimitiveTopology primitiveTopologyToVk(PrimitiveTopology topology);
   VkPolygonMode polygonModeToVk(PolygonMode mode);
   VkCullModeFlags cullModeToVk(CullMode mode);
   VkSampleCountFlagBits sampleCountToVk(uint32 count);
   VkColorComponentFlags colorMaskToVk(uint32 mask);
   VkBlendFactor blendingFactorToVk(BlendingFactor factor);
   VkBlendOp blendingOperationToVk(BlendingOperation operation);
   VkCompareOp testFunctionToVk(TestFunction function);
   VkStencilOp stencilOperationsToVk(StencilOperation operation);
   VkAttachmentLoadOp loadOperationToVk(LoadOperation operation);
   VkAttachmentStoreOp storeOperationToVk(StoreOperation operation);
   VkImageLayout textureLayoutToVk(TextureLayout layout);
}