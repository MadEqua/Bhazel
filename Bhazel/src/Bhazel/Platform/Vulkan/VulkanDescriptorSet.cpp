#include "bzpch.h"

#include "VulkanDescriptorSet.h"
#include "Bhazel/Platform/Vulkan/VulkanConversions.h"


namespace BZ {

    VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(DescriptorType type, uint8 shaderStageVisibilityMask) :
        DescriptorSetLayout(type, shaderStageVisibilityMask) {

        VkDescriptorSetLayoutBinding descriptorSetLayoutBinding = {};
        descriptorSetLayoutBinding.binding = 0;
        descriptorSetLayoutBinding.descriptorType = descriptorTypeToVk(type);
        descriptorSetLayoutBinding.descriptorCount = 1;
        descriptorSetLayoutBinding.stageFlags = shaderStageMaskToVk(shaderStageVisibilityMask);
        descriptorSetLayoutBinding.pImmutableSamplers = nullptr; //For image sampling descriptors

        VkDescriptorSetLayoutCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        createInfo.bindingCount = 1;
        createInfo.pBindings = &descriptorSetLayoutBinding;

        BZ_ASSERT_VK(vkCreateDescriptorSetLayout(getGraphicsContext().getDevice(), &createInfo, nullptr, &nativeHandle));
    }

    VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout() {
        vkDestroyDescriptorSetLayout(getGraphicsContext().getDevice(), nativeHandle, nullptr);
    }
}