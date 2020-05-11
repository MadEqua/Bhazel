#pragma once

#include "Graphics/Internal/VulkanIncludes.h"
#include "Graphics/GpuObject.h"


namespace BZ {

    struct DescriptorDesc {
        VkDescriptorType type;
        VkShaderStageFlags shaderStageFlags;
        uint32 arrayCount;
    };

    class DescriptorSetLayout : public GpuObject<VkDescriptorSetLayout> {
    public:
        static Ref<DescriptorSetLayout> create(const std::initializer_list<DescriptorDesc> &descs);

        explicit DescriptorSetLayout(const std::initializer_list<DescriptorDesc> &descs);
        ~DescriptorSetLayout();

        std::vector<DescriptorDesc>& getDescriptorDescs() { return descriptorDescs; }
        uint32 getDescriptorCount() const { return static_cast<uint32>(descriptorDescs.size()); }

    private:
        std::vector<DescriptorDesc> descriptorDescs;
    };


    /*-------------------------------------------------------------------------------------------*/
    class Buffer;
    class TextureView;
    class Sampler;

    /*
    * DescriptorPools create the DescriptorSets.
    */
    class DescriptorSet : public GpuObject<VkDescriptorSet> {
    public:
        static DescriptorSet& get(const Ref<DescriptorSetLayout> &layout);

        BZ_NON_COPYABLE(DescriptorSet);

        void setConstantBuffer(const Ref<Buffer> &buffer, uint32 binding, uint32 offset, uint32 size);
        void setConstantBuffers(const Ref<Buffer> buffers[], uint32 srcArrayCount, uint32 dstArrayOffset, uint32 binding, uint32 offsets[], uint32 sizes[]);

        void setCombinedTextureSampler(const Ref<TextureView> &textureView, const Ref<Sampler> &sampler, uint32 binding);
        void setCombinedTextureSamplers(const Ref<TextureView> textureViews[], uint32 srcArrayCount, uint32 dstArrayOffset, const Ref<Sampler> &sampler, uint32 binding);

        void setSampledTexture(const Ref<TextureView> &textureView, uint32 binding);
        void setSampledTextures(const Ref<TextureView> textureViews[], uint32 srcArrayCount, uint32 dstArrayOffset, uint32 binding);

        void setSampler(const Ref<Sampler>& sampler, uint32 binding);
        void setSamplers(const Ref<Sampler> samplers[], uint32 srcArrayCount, uint32 dstArrayOffset, uint32 binding);

        //Helper for filling dynamic Buffer offsets when binding the DescriptorSet.
        struct DynBufferData {
            DynBufferData(uint32 binding, const Ref<Buffer> *buffers, uint32 arrayCount);
            uint32 binding;
            std::vector<Ref<Buffer>> buffers;
        };

        const DynBufferData* getDynamicBufferDataByBinding(uint32 binding) const;
        uint32 getDynamicBufferCount() const;

        Ref<DescriptorSetLayout> getLayout() const { return layout; }

    private:
        DescriptorSet() = default;
        void init(VkDescriptorSet vkDescriptorSet, const Ref<DescriptorSetLayout> &layout);

        Ref<DescriptorSetLayout> layout;

        //Only storing Buffers (dynamic) and not other descriptors because that's the only type that has the need (for automatic dynamic buffer offset filling by the engine).
        std::vector<DynBufferData> dynamicBuffers;

        friend class DescriptorPool;
    };
}
