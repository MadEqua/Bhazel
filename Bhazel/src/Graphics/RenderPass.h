#pragma once

#include "Texture.h"
#include "Graphics/Color.h"


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
        TextureFormat format = TextureFormatEnum::B8G8R8A8;
        uint32 samples = 1;
        LoadOperation loadOperatorColorAndDepth = LoadOperation::DontCare;
        StoreOperation storeOperatorColorAndDepth = StoreOperation::Store;
        LoadOperation loadOperatorStencil = LoadOperation::Clear;
        StoreOperation storeOperatorStencil = StoreOperation::Store;
        TextureLayout initialLayout = TextureLayout::Undefined;
        TextureLayout finalLayout = TextureLayout::Undefined;
        ClearValues clearValues = {}; //RGBA or Depth/Stencil.
    };


    class RenderPass {
    public:
        static Ref<RenderPass> create(const std::initializer_list<AttachmentDescription> &descs);

        uint32 getColorAttachmentCount() const { return static_cast<uint32>(colorAttachmentDescs.size()); }
        uint32 getAttachmentCount() const { return static_cast<uint32>(colorAttachmentDescs.size()) + (depthStencilAttachmentDesc.has_value() ? 1 : 0); }
        bool hasDepthStencilAttachment() const { return depthStencilAttachmentDesc.has_value(); }

        const std::vector<AttachmentDescription>& getColorAttachmentDescriptions() const;
        const AttachmentDescription& getColorAttachmentDescription(uint32 index) const;
        const AttachmentDescription* getDepthStencilAttachmentDescription() const;

        explicit RenderPass(const std::initializer_list<AttachmentDescription> &descs);
        virtual ~RenderPass() = default;

    protected:
        std::vector<AttachmentDescription> colorAttachmentDescs;

        //Can be Depth only or DepthStencil
        std::optional<AttachmentDescription> depthStencilAttachmentDesc;
    };


}