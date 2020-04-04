#include "bzpch.h"

#include "Material.h"
#include "Renderer.h"


namespace BZ {

    Material::Material(Ref<Texture2D> &albedoTexture, Ref<Texture2D> &normalTexture, Ref<Texture2D> &metallicTexture, Ref<Texture2D> &roughnessTextureView) :
        albedoTextureView(TextureView::create(albedoTexture)),
        normalTextureView(TextureView::create(normalTexture)),
        metallicTextureView(TextureView::create(metallicTexture)),
        roughnessTextureView(TextureView::create(roughnessTextureView)) {
        init();
    }

    Material::Material(Ref<Texture2D> &albedoTexture, Ref<Texture2D> &normalTexture) :
        albedoTextureView(TextureView::create(albedoTexture)),
        normalTextureView(TextureView::create(normalTexture)) {
        init();
    }

    Material::Material(Ref<TextureCube> &albedoTexture):
        albedoTextureView(TextureView::create(albedoTexture)) {
        init();
    }

    Material::Material(const char *albedoTexturePath, const char *normalTexturePath, const char *metallicTexturePath, const char *roughnessTexturePath) {
        auto albedoTexture = Texture2D::create(albedoTexturePath, TextureFormat::R8G8B8A8_SRGB, true);
        albedoTextureView = TextureView::create(albedoTexture);

        auto normalTexture = Texture2D::create(normalTexturePath, TextureFormat::R8G8B8A8, true);
        normalTextureView = TextureView::create(normalTexture);

        auto metallicTexture = Texture2D::create(metallicTexturePath, TextureFormat::R8G8B8A8, true);
        metallicTextureView = TextureView::create(metallicTexture);

        auto roughnessTexture = Texture2D::create(roughnessTexturePath, TextureFormat::R8G8B8A8, true);
        roughnessTextureView = TextureView::create(roughnessTexture);

        init();
    }

    Material::Material(const char *albedoTexturePath, const char *normalTexturePath) {
        auto albedoTexture = Texture2D::create(albedoTexturePath, TextureFormat::R8G8B8A8_SRGB, true);
        albedoTextureView = TextureView::create(albedoTexture);

        auto normalTexture = Texture2D::create(normalTexturePath, TextureFormat::R8G8B8A8, true);
        normalTextureView = TextureView::create(normalTexture);

        init();
    }

    void Material::init() {
        descriptorSet = DescriptorSet::create(Renderer::getMaterialDescriptorSetLayout());
        descriptorSet->setCombinedTextureSampler(albedoTextureView, Renderer::getDefaultSampler(), 0);

        if(normalTextureView)
            descriptorSet->setCombinedTextureSampler(normalTextureView, Renderer::getDefaultSampler(), 1);

        if(metallicTextureView)
            descriptorSet->setCombinedTextureSampler(metallicTextureView, Renderer::getDefaultSampler(), 2);

        if(roughnessTextureView)
            descriptorSet->setCombinedTextureSampler(roughnessTextureView, Renderer::getDefaultSampler(), 3);
    }
}