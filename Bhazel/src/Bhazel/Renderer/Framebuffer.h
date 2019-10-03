#pragma once

#include "Bhazel/Renderer/Texture.h"


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

    struct AttachmentDescription {
        TextureFormat format = TextureFormatEnum::R8G8B8;
        uint32 samples = 1;
        LoadOperation loadOperatorColorAndDepth = LoadOperation::Clear;
        StoreOperation storeOperatorColorAndDepth = StoreOperation::Store;
        LoadOperation loadOperatorStencil = LoadOperation::Clear;
        StoreOperation storeOperatorStencil = StoreOperation::Store;
        TextureLayout initialLayout = TextureLayout::Undefined;
        TextureLayout finalLayout = TextureLayout::Undefined;
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
        };

        virtual ~Framebuffer() = default;

        uint32 getColorAttachmentCount() const { return static_cast<uint32>(colorAttachments.size()); }
        bool hasDepthStencilAttachment() const { return depthStencilAttachment.has_value(); }

    protected:
        Framebuffer(const std::vector<Attachment> &attachments, const glm::ivec3 &dimensions);

        std::vector<Attachment> colorAttachments;
        std::optional<Attachment> depthStencilAttachment;
        glm::ivec3 dimensions;
    };
}