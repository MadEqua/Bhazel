#pragma once


namespace BZ {

    class Buffer;
    class TextureView;
    class Sampler;

    enum class ShaderStageFlag {
        Vertex = 1,
        TesselationControl = 2,
        TesselationEvaluation = 4,
        Geometry = 8,
        Fragment = 16,
        Compute = 32,
        GraphicsAll = Vertex | TesselationControl | TesselationEvaluation | Geometry | Fragment,
        All = GraphicsAll | Compute,
    };

    EnumClassFlagOperators(ShaderStageFlag);

    enum class DescriptorType {
        Sampler,
        CombinedTextureSampler, //Texture + Sampler
        SampledTexture,
        StorageTexture,
        ConstantTexelBuffer,
        StorageTexelBuffer,
        ConstantBuffer,
        StorageBuffer,
        ConstantBufferDynamic,
        StorageBufferDynamic,
        InputAttachment,

        Count
    };


    class DescriptorSetLayout {
    public:
        struct DescriptorDesc {
            DescriptorType type;
            uint8 shaderStageVisibililtyMask;
            uint32 arrayCount;
        };

        class Builder {
        public:
            Builder& addDescriptorDesc(DescriptorType type, uint8 shaderStageVisibilityMask, uint32 arrayCount);
            Ref<DescriptorSetLayout> build() const;
            void reset() { descriptorDescs.clear(); }

        private:
            std::vector<DescriptorDesc> descriptorDescs; //By order of binding

            friend class DescriptorSetLayout;
            friend class VulkanDescriptorSetLayout;
        };

        std::vector<DescriptorDesc>& getDescriptorDescs() { return descriptorDescs; }
        uint32 getDescriptorCount() const { return static_cast<uint32>(descriptorDescs.size()); }

    protected:
        explicit DescriptorSetLayout(const Builder &builder);
        virtual ~DescriptorSetLayout() = default;

        std::vector<DescriptorDesc> descriptorDescs;
    };


    class DescriptorSet {
    public:
        static Ref<DescriptorSet> create(const Ref<DescriptorSetLayout> &layout);

        void setConstantBuffer(const Ref<Buffer> &buffer, uint32 binding, uint32 offset, uint32 size);
        void setConstantBuffers(const Ref<Buffer> buffers[], uint32 srcArrayCount, uint32 dstArrayOffset, uint32 binding, uint32 offsets[], uint32 sizes[]);

        void setCombinedTextureSampler(const Ref<TextureView> &textureView, const Ref<Sampler> &sampler, uint32 binding);
        void setCombinedTextureSamplers(const Ref<TextureView> textureViews[], uint32 srcArrayCount, uint32 dstArrayOffset, const Ref<Sampler> &sampler, uint32 binding);

        void setSampledTexture(const Ref<TextureView> &textureView, uint32 binding);

        void setSampler(const Ref<Sampler>& sampler, uint32 binding);

        struct DynBufferData {
            DynBufferData(uint32 binding, const Ref<Buffer> *buffers, uint32 arrayCount) : 
                binding(binding), arrayCount(arrayCount) {
                for(uint32 i = 0; i < arrayCount; ++i)
                    this->buffers.push_back(buffers[i]);
            }
            uint32 binding;
            std::vector<Ref<Buffer>> buffers;
            uint32 arrayCount;
        };
        const DynBufferData* getDynamicBufferDataByBinding(uint32 binding) const;

        uint32 getDynamicBufferCount() const {
            uint32 count = 0;
            for (const auto &data : dynamicBuffers) {
                count += data.arrayCount;
            }
            return count;

        }
        Ref<DescriptorSetLayout> getLayout() const { return layout; }

    protected:
        explicit DescriptorSet(const Ref<DescriptorSetLayout> &layout);

        Ref<DescriptorSetLayout> layout;

        //Only storing Buffers (dynamic) and not other descriptors because that's the only type that has the need (for automatic dynamic buffer offset filling by the engine).
        std::vector<DynBufferData> dynamicBuffers;

        virtual void internalSetConstantBuffer(const Ref<Buffer> *buffers, uint32 srcArrayCount, uint32 dstArrayOffset, uint32 binding, uint32 *offsets, uint32 *sizes) = 0;
        virtual void internalSetCombinedTextureSampler(const Ref<TextureView> *textureViews, uint32 srcArrayCount, uint32 dstArrayOffset, const Ref<Sampler> &sampler, uint32 binding) = 0;
        virtual void internalSetSampledTexture(const Ref<TextureView> *textureViews, uint32 srcArrayCount, uint32 dstArrayOffset, uint32 binding) = 0;
        virtual void internalSetSampler(const Ref<Sampler> *samplers, uint32 srcArrayCount, uint32 dstArrayOffset, uint32 binding) = 0;
    };
}
