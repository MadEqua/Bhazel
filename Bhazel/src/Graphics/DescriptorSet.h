#pragma once


namespace BZ {

    class Buffer;

    enum class ShaderStageFlags {
        Vertex = 1,
        TesselationControl = 2,
        TesselationEvaluation = 4,
        Geometry = 8,
        Fragment = 16,
        Compute = 32,
        GraphicsAll = Vertex | TesselationControl | TesselationEvaluation | Geometry | Fragment,
        All = GraphicsAll | Compute,
    };

    EnumClassFlagOperators(ShaderStageFlags);

    enum class DescriptorType {
        Sampler,
        CombinedTextureSampler,
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
    protected:
        struct DescriptorDesc {
            DescriptorType type;
            uint8 shaderStageVisibililtyMask;
            uint32 arrayCount;
        };

    public:
        class Builder {
        public:
            Builder &addDescriptorDesc(DescriptorType type, uint8 shaderStageVisibilityMask, uint32 arrayCount);
            Ref<DescriptorSetLayout> build() const;

        private:
            std::vector<DescriptorDesc> descriptorDescs; //By order of binding

            friend class DescriptorSetLayout;
            friend class VulkanDescriptorSetLayout;
        };

        const std::vector<DescriptorDesc> &getDescriptorDescs() const { return descriptorDescs; }

    protected:
        explicit DescriptorSetLayout(const Builder &builder);
        virtual ~DescriptorSetLayout() = default;

        std::vector<DescriptorDesc> descriptorDescs;
    };


    class DescriptorSet {
    public:
        static Ref<DescriptorSet> create(const Ref<DescriptorSetLayout> &layout);

        virtual void setConstantBuffer(const Ref<Buffer> &buffer, uint32 binding, uint32 offset, uint32 size) = 0;
    protected:
        explicit DescriptorSet(const Ref<DescriptorSetLayout> &layout);

        Ref<DescriptorSetLayout> layout;
    };
}
