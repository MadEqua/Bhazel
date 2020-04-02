#pragma once

#include "Graphics/Texture.h"
#include "Graphics/DescriptorSet.h"


namespace BZ {

    class Material {
    public:
        Material() = default;
        explicit Material(Ref<Texture2D> &albedoTexture);
        explicit Material(Ref<TextureCube> &albedoTexture);
        explicit Material(const char *albedoTexturePath);

        bool isValid() const { return static_cast<bool>(descriptorSet); }

        const Ref<TextureView>& getAlbedoTextureView() const { return albedoTextureView; }
        const Ref<DescriptorSet>& getDescriptorSet() const { return descriptorSet; }

    private:
        Ref<TextureView> albedoTextureView;

        Ref<DescriptorSet> descriptorSet;

        void init();
    };
}