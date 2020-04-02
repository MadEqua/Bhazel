#include "bzpch.h"

#include "Material.h"
#include "Renderer.h"


namespace BZ {

    Material::Material(Ref<Texture2D> &albedoTexture) :
        albedoTextureView(TextureView::create(albedoTexture)) {
        init();
    }

    Material::Material(Ref<TextureCube> &albedoTexture):
        albedoTextureView(TextureView::create(albedoTexture)) {
        init();
    }

    Material::Material(const char *albedoTexturePath) {
        auto albedoTexture = Texture2D::create(albedoTexturePath, TextureFormat::R8G8B8A8_SRGB, true);
        albedoTextureView = TextureView::create(albedoTexture);
        init();
    }

    void Material::init() {
        descriptorSet = DescriptorSet::create(Renderer::getMaterialDescriptorSetLayout());
        descriptorSet->setCombinedTextureSampler(albedoTextureView, Renderer::getDefaultSampler(), 0);
    }
}