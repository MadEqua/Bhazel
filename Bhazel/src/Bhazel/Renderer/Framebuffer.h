#pragma once

#include "Bhazel/Renderer/Texture.h"


namespace BZ {

    class Framebuffer {
    public:
        static Ref<Framebuffer> create(const std::vector<Ref<TextureView>> &textureViews);

        virtual ~Framebuffer() = default;

        uint32 getColorAttachmentCount() const { return static_cast<uint32>(textureViews.size()); } //TODO

    protected:
        Framebuffer(const std::vector<Ref<TextureView>> &textureViews);

        std::vector<Ref<TextureView>> textureViews;
    };
}