#include "bzpch.h"

#include "Material.h"
#include "Renderer.h"


namespace BZ {

    Material::Material(Ref<Texture2D> &albedoTexture, Ref<Texture2D> &normalTexture, Ref<Texture2D> &metallicTexture, Ref<Texture2D> &roughnessTextureView, Ref<Texture2D> &heightTextureView) :
        albedoTextureView(TextureView::create(albedoTexture)),
        normalTextureView(TextureView::create(normalTexture)),
        metallicTextureView(TextureView::create(metallicTexture)),
        roughnessTextureView(TextureView::create(roughnessTextureView)),
        heightTextureView(TextureView::create(heightTextureView)) {
        init();
    }

    Material::Material(Ref<Texture2D> &albedoTexture, Ref<Texture2D> &normalTexture) :
        albedoTextureView(TextureView::create(albedoTexture)),
        normalTextureView(TextureView::create(normalTexture)) {
        init();
    }

    Material::Material(Ref<TextureCube> &albedoTexture) :
        albedoTextureView(TextureView::create(albedoTexture)) {
        init();
    }

    Material::Material(const char *albedoTexturePath, const char *normalTexturePath, const char *metallicTexturePath, const char *roughnessTexturePath, const char *heightTexturePath) {
        auto albedoTexture = Texture2D::create(albedoTexturePath, TextureFormat::R8G8B8A8_SRGB, MipmapData::Options::Generate);
        albedoTextureView = TextureView::create(albedoTexture);

        auto normalTexture = Texture2D::create(normalTexturePath, TextureFormat::R8G8B8A8, MipmapData::Options::Generate);
        normalTextureView = TextureView::create(normalTexture);

        auto metallicTexture = Texture2D::create(metallicTexturePath, TextureFormat::R8, MipmapData::Options::Generate);
        metallicTextureView = TextureView::create(metallicTexture);

        auto roughnessTexture = Texture2D::create(roughnessTexturePath, TextureFormat::R8, MipmapData::Options::Generate);
        roughnessTextureView = TextureView::create(roughnessTexture);

        auto heightTexture = Texture2D::create(heightTexturePath, TextureFormat::R8, MipmapData::Options::Generate);
        heightTextureView = TextureView::create(heightTexture);

        init();
    }

    Material::Material(const char *albedoTexturePath, const char *normalTexturePath) {
        auto albedoTexture = Texture2D::create(albedoTexturePath, TextureFormat::R8G8B8A8_SRGB, MipmapData::Options::Generate);
        albedoTextureView = TextureView::create(albedoTexture);

        auto normalTexture = Texture2D::create(normalTexturePath, TextureFormat::R8G8B8A8, MipmapData::Options::Generate);
        normalTextureView = TextureView::create(normalTexture);

        init();
    }

    void Material::init() {
        descriptorSet = Renderer::createMaterialDescriptorSet();
        descriptorSet->setCombinedTextureSampler(albedoTextureView, Renderer::getDefaultSampler(), 1);

        if(normalTextureView)
            descriptorSet->setCombinedTextureSampler(normalTextureView, Renderer::getDefaultSampler(), 2);

        if(metallicTextureView)
            descriptorSet->setCombinedTextureSampler(metallicTextureView, Renderer::getDefaultSampler(), 3);

        if(roughnessTextureView)
            descriptorSet->setCombinedTextureSampler(roughnessTextureView, Renderer::getDefaultSampler(), 4);

        if (heightTextureView)
            descriptorSet->setCombinedTextureSampler(heightTextureView, Renderer::getDefaultSampler(), 5);
    }

    bool Material::operator==(const Material &other) const {
        return albedoTextureView == other.albedoTextureView &&
            normalTextureView == other.normalTextureView &&
            metallicTextureView == other.metallicTextureView &&
            roughnessTextureView == other.roughnessTextureView &&
            heightTextureView == other.heightTextureView &&
            parallaxOcclusionScale == other.parallaxOcclusionScale;
    }
}