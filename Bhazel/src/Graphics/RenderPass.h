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
        DepthReadOnlyStencilAttachmentOptimal,
        DepthAttachmentStencilReadOnlyOptimal,
        DepthAttachmentOptimal,
        DepthReadOnlyOptimal,
        StencilAttachmentOptimal,
        StencilReadOnlyOptimal,
        Preinitialized,
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

    enum class PipelineStageFlag {
        TopOfPipe =             0b1,
        VertexInput =           0b10,
        VertexShader =          0b100,
        GeometryShader =        0b1000,
        FragmentShader =        0b10000,
        EarlyFragmentTests =    0b100000,
        LateFragmentTests =     0b1000000,
        ColorAttachmentOutput = 0b10000000,
        BottomOfPipe =          0b100000000,
        AllGraphics =           0b1000000000,
        AllCommands =           0b10000000000
    };

    EnumClassFlagOperators(PipelineStageFlag);

    enum class AccessFlag {
        IndirectCommandRead =           0b1,
        IndexRead =                     0b10,
        VertexAttrRead =                0b100,
        UniformRead =                   0b1000,
        InputAttachmentRead =           0b10000,
        ShaderRead =                    0b100000,
        ShaderWrite =                   0b1000000,
        ColorAttachmentRead =           0b10000000,
        ColorAttachmentWrite =          0b100000000,
        DepthStencilAttachmentRead =    0b1000000000,
        DepthStencilAttachmentWrite =   0b10000000000,
        TransferRead =                  0b100000000000,
        TransferWrite =                 0b1000000000000,
        HostRead =                      0b10000000000000,
        HostWrite =                     0b100000000000000,
        MemoryRead =                    0b10000000000000000,
        MemoryWrite =                   0b100000000000000000
    };

    EnumClassFlagOperators(AccessFlag);

    enum class DependencyFlag {
        ByRegion =          1,
        DeviceGroup =       2,
        ViewLocal =         4
    };

    EnumClassFlagOperators(DependencyFlag);


    struct SubPassDependency {
        //Indices refer to parent RenderPass SubPass list.
        uint32 srcSubPassIndex;
        uint32 dstSubPassIndex;

        uint8 srcStageMask;
        uint8 dstStageMask;

        uint8 srcAccessMask;
        uint8 dstAccessMask;

        uint8 dependencyFlags;
    };

    struct AttachmentReference {
        //Index refers to parent RenderPass AttachmentDescription list.
        uint32 index;

        //Layout the attachment uses during the SubPass.
        TextureLayout layout;
    };

    struct SubPassDescription {
        
        //TODO: not implemented
        //std::vector<uint32> inputAttachmentsRefs;
        //std::vector<uint32> resolveAttachmentsRefs;

        //Order in this list correspond to fragment shader outputs.
        std::vector<AttachmentReference> colorAttachmentsRefs;
        std::optional<AttachmentReference> depthStencilAttachmentsRef;
        std::vector<uint32> preserveAttachmentsIndices;
    };

    class RenderPass {
    public:
        static Ref<RenderPass> create(const std::initializer_list<AttachmentDescription> &descs, 
                                      const std::initializer_list<SubPassDescription> &subPassDescs,
                                      const std::initializer_list<SubPassDependency> &subPassDeps = {});

        uint32 getAttachmentCount() const { return static_cast<uint32>(attachmentDescs.size()); }
        uint32 getColorAttachmentCount() const { return static_cast<uint32>(colorAttachmentIndices.size()); }
        bool hasDepthStencilAttachment() const { return depthStencilAttachmentDescIndex.has_value(); }
        
        uint32 getSubPassCount() const { return static_cast<uint32>(subPassDescs.size()); }

        const AttachmentDescription& getColorAttachmentDescription(uint32 index) const;
        const AttachmentDescription* getDepthStencilAttachmentDescription() const;

        RenderPass(const std::initializer_list<AttachmentDescription> &descs, 
                   const std::initializer_list<SubPassDescription> &subPassDescs,
                   const std::initializer_list<SubPassDependency> &subPassDeps);
        virtual ~RenderPass() = default;

    protected:
        std::vector<AttachmentDescription> attachmentDescs;
        std::vector<SubPassDescription> subPassDescs;
        std::vector<SubPassDependency> subPassDeps;

        //Indices refer to the main AttachmentDescription list.
        std::vector<uint32> colorAttachmentIndices;

        //Can be Depth only or DepthStencil
        std::optional<uint32> depthStencilAttachmentDescIndex;

    };
}