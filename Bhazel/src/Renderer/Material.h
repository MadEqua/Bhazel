#pragma once

#include "Graphics/Texture.h"
#include "Graphics/DescriptorSet.h"


namespace BZ {

    class Material {
    public:
        Material() = default;
        Material(Ref<Texture2D> &albedoTexture);
        Material(const char *albedoTexturePath);

        const Ref<TextureView>& getAlbedoTextureView() const { return albedoTextureView; }
        const Ref<DescriptorSet>& getDescriptorSet() const { return descriptorSet; }

    private:
        Ref<TextureView> albedoTextureView;

        Ref<DescriptorSet> descriptorSet;

        void init();
    };
}