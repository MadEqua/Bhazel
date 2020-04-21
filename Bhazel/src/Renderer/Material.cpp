#include "bzpch.h"

#include "Material.h"
#include "Renderer.h"


namespace BZ {

    Material::Material(Ref<Texture2D> &albedoTexture, Ref<Texture2D> &normalTexture,
                       Ref<Texture2D> &metallicTexture, Ref<Texture2D> &roughnessTexture,
                       Ref<Texture2D> &heightTexture, Ref<Texture2D> &aoTexture) :
        albedoTextureView(TextureView::create(albedoTexture)) {

        if (normalTexture) normalTextureView = TextureView::create(normalTexture);
        if (metallicTexture) metallicTextureView = TextureView::create(metallicTexture);
        if (roughnessTexture) roughnessTextureView = TextureView::create(roughnessTexture);
        if (heightTexture) heightTextureView = TextureView::create(heightTexture);
        if (aoTexture) aoTextureView = TextureView::create(aoTexture);

        init();
    }

    Material::Material(Ref<TextureCube> &albedoTexture) :
        albedoTextureView(TextureView::create(albedoTexture)) {
        init();
    }

    Material::Material(const char *albedoTexturePath, const char *normalTexturePath,
                       const char *metallicTexturePath, const char *roughnessTexturePath,
                       const char *heightTexturePath, const char *aoTexturePath) {
        auto albedoTexture = Texture2D::create(albedoTexturePath, TextureFormatEnum::R8G8B8A8_SRGB, MipmapData::Options::Generate);
        albedoTextureView = TextureView::create(albedoTexture);

        if (normalTexturePath) {
            auto normalTexture = Texture2D::create(normalTexturePath, TextureFormatEnum::R8G8B8A8, MipmapData::Options::Generate);
            normalTextureView = TextureView::create(normalTexture);
        }

        if (metallicTexturePath) {
            auto metallicTexture = Texture2D::create(metallicTexturePath, TextureFormatEnum::R8, MipmapData::Options::Generate);
            metallicTextureView = TextureView::create(metallicTexture);
        }

        if (roughnessTexturePath) {
            auto roughnessTexture = Texture2D::create(roughnessTexturePath, TextureFormatEnum::R8, MipmapData::Options::Generate);
            roughnessTextureView = TextureView::create(roughnessTexture);
        }

        if (heightTexturePath) {
            auto heightTexture = Texture2D::create(heightTexturePath, TextureFormatEnum::R8, MipmapData::Options::Generate);
            heightTextureView = TextureView::create(heightTexture);
        }

        if (aoTexturePath) {
            auto aoTexture = Texture2D::create(aoTexturePath, TextureFormatEnum::R8, MipmapData::Options::Generate);
            aoTextureView = TextureView::create(aoTexture);
        }

        init();
    }

    void Material::init() {
        descriptorSet = Renderer::createMaterialDescriptorSet();
        descriptorSet->setCombinedTextureSampler(albedoTextureView, Renderer::getDefaultSampler(), 1);

        if(normalTextureView)
            descriptorSet->setCombinedTextureSampler(normalTextureView, Renderer::getDefaultSampler(), 2);
        else
            descriptorSet->setCombinedTextureSampler(Renderer::getDummyTextureView(), Renderer::getDefaultSampler(), 2);

        if (metallicTextureView)
            descriptorSet->setCombinedTextureSampler(metallicTextureView, Renderer::getDefaultSampler(), 3);
        else
            descriptorSet->setCombinedTextureSampler(Renderer::getDummyTextureView(), Renderer::getDefaultSampler(), 3);

        if(roughnessTextureView)
            descriptorSet->setCombinedTextureSampler(roughnessTextureView, Renderer::getDefaultSampler(), 4);
        else
            descriptorSet->setCombinedTextureSampler(Renderer::getDummyTextureView(), Renderer::getDefaultSampler(), 4);

        if (heightTextureView)
            descriptorSet->setCombinedTextureSampler(heightTextureView, Renderer::getDefaultSampler(), 5);
        else
            descriptorSet->setCombinedTextureSampler(Renderer::getDummyTextureView(), Renderer::getDefaultSampler(), 5);

        if (aoTextureView)
            descriptorSet->setCombinedTextureSampler(aoTextureView, Renderer::getDefaultSampler(), 6);
        else
            descriptorSet->setCombinedTextureSampler(Renderer::getDummyTextureView(), Renderer::getDefaultSampler(), 6);
    }

    bool Material::operator==(const Material &other) const {
        return albedoTextureView == other.albedoTextureView &&
            normalTextureView == other.normalTextureView &&
            metallicTextureView == other.metallicTextureView &&
            roughnessTextureView == other.roughnessTextureView &&
            heightTextureView == other.heightTextureView &&
            aoTextureView == other.aoTextureView &&
            parallaxOcclusionScale == other.parallaxOcclusionScale;
    }
}