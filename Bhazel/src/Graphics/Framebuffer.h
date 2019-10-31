#pragma once

#include "Graphics/Texture.h"


namespace BZ {

    enum class LoadOperation {
        Load,
        Clear,
        DontCare
    };

    enum class StoreOperation {
        Store,
        DontCare
    };

    enum class TextureLayout {
        Undefined,
        General,
        ColorAttachmentOptimal,
        DepthStencilAttachmentOptimal,
        DepthStencilReadOnlyOptimal,
        ShaderReadOnlyOptimal,
        TransferSrcOptimal,
        TransferDstOptimal,
        Preinitialized,
        DepthReadOnlyStencilAttachmentOptimal,
        DepthAttachmentStencilReadOnlyOptimal,
        Present,
        SharedPresent
    };

    union ClearValues {
        glm::vec4 floating;
        glm::ivec4 integer;
    };

    struct AttachmentDescription {
        TextureFormat format = TextureFormatEnum::R8G8B8;
        uint32 samples = 1;
        LoadOperation loadOperatorColorAndDepth = LoadOperation::Clear;
        StoreOperation storeOperatorColorAndDepth = StoreOperation::Store;
        LoadOperation loadOperatorStencil = LoadOperation::Clear;
        StoreOperation storeOperatorStencil = StoreOperation::Store;
        TextureLayout initialLayout = TextureLayout::Undefined;
        TextureLayout finalLayout = TextureLayout::Undefined;
        ClearValues clearValues = {}; //RGBA or Depth/Stencil.
    };


    //Supports Color and DepthStencil attachments only.
    class Framebuffer {
    protected:
        struct Attachment {
            AttachmentDescription description;
            Ref<TextureView> textureView;
        };

    public:
        class Builder {
        public:
            Builder& addColorAttachment(const AttachmentDescription &desc, const Ref<TextureView> &textureView);
            Builder& addDepthStencilAttachment(const AttachmentDescription &desc, const Ref<TextureView> &textureView);
            Builder& setDimensions(const glm::ivec3 &dimensions);
            Ref<Framebuffer> build() const;

        private:
            std::vector<Attachment> attachments;
            glm::ivec3 dimensions = {1,1,1};

            friend class Framebuffer;
            friend class VulkanFramebuffer;
        };

        uint32 getColorAttachmentCount() const { return static_cast<uint32>(colorAttachments.size()); }
        uint32 getAttachmentCount() const { return static_cast<uint32>(colorAttachments.size()) + (depthStencilAttachment.has_value() ? 1 : 0); }
        bool hasDepthStencilAttachment() const { return depthStencilAttachment.has_value(); }
        const glm::ivec3& getDimensions() const { return dimensions; }

        const Attachment& getColorAttachment(uint32 index) const;
        const Attachment* getDepthStencilAttachment() const;

    protected:
        explicit Framebuffer(const Builder &builder);
        virtual ~Framebuffer() = default;

        std::vector<Attachment> colorAttachments;
        std::optional<Attachment> depthStencilAttachment;
        glm::ivec3 dimensions;
    };
}