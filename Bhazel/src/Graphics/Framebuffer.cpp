#include "bzpch.h"

#include "Framebuffer.h"

#include "Graphics/Graphics.h"

#include "Platform/Vulkan/VulkanFramebuffer.h"


namespace BZ {

    Framebuffer::Builder& Framebuffer::Builder::addColorAttachment(const AttachmentDescription &desc, const Ref<TextureView> &textureView) {
        BZ_ASSERT_CORE(textureView->getTextureFormat().isColor(), "TextureView needs to have a color format!");
        attachments.push_back({ desc, textureView });
        return *this;
    }

    Framebuffer::Builder& Framebuffer::Builder::addDepthStencilAttachment(const AttachmentDescription &desc, const Ref<TextureView> &textureView) {
        BZ_ASSERT_CORE(textureView->getTextureFormat().isDepthStencil(), "TextureView needs to have a DepthStencil format!");
        attachments.push_back({ desc, textureView });
        return *this;
    }

    Framebuffer::Builder &Framebuffer::Builder::setDimensions(const glm::ivec3 &dimensions) {
        this->dimensions = dimensions;
        return *this;
    }

    Ref<Framebuffer> Framebuffer::Builder::build() const {
        switch(Graphics::api) {
            /*case Graphics::API::OpenGL:
                return MakeRef<OpenGLTexture2D>(assetsPath + path);
            case Graphics::API::D3D11:
                return MakeRef<D3D11Texture2D>(assetsPath + path);*/
        case Graphics::API::Vulkan:
            return MakeRef<VulkanFramebuffer>(*this);
        default:
            BZ_ASSERT_ALWAYS_CORE("Unknown RendererAPI.");
            return nullptr;
        }
    }


    Framebuffer::Framebuffer(const Builder &builder) :
        dimensions(builder.dimensions) {
        BZ_ASSERT_CORE(!builder.attachments.empty(), "Creating a Framebuffer with no attachments!");

        for(const auto &att : builder.attachments) {
            if(att.textureView->getTextureFormat().isColor()) {
                colorAttachments.push_back(att);
            }
            else if(att.textureView->getTextureFormat().isDepthStencil()) {
                BZ_ASSERT_CORE(!depthStencilAttachment, "Adding more than one DepthStencilAttachment!");
                depthStencilAttachment.emplace(att);
            }
        }
    }

    const Framebuffer::Attachment& Framebuffer::getColorAttachment(uint32 index) const {
        BZ_ASSERT_CORE(index < getColorAttachmentCount(), "Index {} is out of range!", index)
        return colorAttachments[index];
    }

    const Framebuffer::Attachment* Framebuffer::getDepthStencilAttachment() const {
        if(depthStencilAttachment.has_value())
            return &depthStencilAttachment.value();
        return nullptr;
    }
}