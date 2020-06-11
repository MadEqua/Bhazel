#include "bzpch.h"

#include "Material.h"
#include "Renderer.h"

#include "Core/Engine.h"
#include "Graphics/DescriptorSet.h"
#include "Graphics/GraphicsContext.h"
#include "Graphics/Texture.h"


namespace BZ {

Material::Material(Ref<Texture2D> &albedoTexture, Ref<Texture2D> &normalTexture, Ref<Texture2D> &metallicTexture,
                   Ref<Texture2D> &roughnessTexture, Ref<Texture2D> &heightTexture, Ref<Texture2D> &aoTexture,
                   bool useAnisotropicSampler) :
    albedoTextureView(TextureView::create(albedoTexture)),
    anisotropicSampler(useAnisotropicSampler) {

    if (normalTexture)
        normalTextureView = TextureView::create(normalTexture);
    if (metallicTexture)
        metallicTextureView = TextureView::create(metallicTexture);
    if (roughnessTexture)
        roughnessTextureView = TextureView::create(roughnessTexture);
    if (heightTexture)
        heightTextureView = TextureView::create(heightTexture);
    if (aoTexture)
        aoTextureView = TextureView::create(aoTexture);

    init();
}

Material::Material(Ref<TextureCube> &albedoTexture) : albedoTextureView(TextureView::create(albedoTexture)) {
    init();
}

Material::Material(const char *albedoTexturePath, const char *normalTexturePath, const char *metallicTexturePath,
                   const char *roughnessTexturePath, const char *heightTexturePath, const char *aoTexturePath,
                   bool useAnisotropicSampler) :
    anisotropicSampler(useAnisotropicSampler) {

    auto albedoTexture = Texture2D::create(albedoTexturePath, VK_FORMAT_R8G8B8A8_SRGB, MipmapData::Options::Generate);
    BZ_SET_TEXTURE_DEBUG_NAME(albedoTexture, "Material Albedo Texture");
    albedoTextureView = TextureView::create(albedoTexture);

    if (normalTexturePath) {
        auto normalTexture =
            Texture2D::create(normalTexturePath, VK_FORMAT_R8G8B8A8_UNORM, MipmapData::Options::Generate);
        BZ_SET_TEXTURE_DEBUG_NAME(normalTexture, "Material Normal Texture");
        normalTextureView = TextureView::create(normalTexture);
    }

    if (metallicTexturePath) {
        auto metallicTexture =
            Texture2D::create(metallicTexturePath, VK_FORMAT_R8_UNORM, MipmapData::Options::Generate);
        BZ_SET_TEXTURE_DEBUG_NAME(metallicTexture, "Material Metallic Texture");
        metallicTextureView = TextureView::create(metallicTexture);
    }

    if (roughnessTexturePath) {
        auto roughnessTexture =
            Texture2D::create(roughnessTexturePath, VK_FORMAT_R8_UNORM, MipmapData::Options::Generate);
        BZ_SET_TEXTURE_DEBUG_NAME(roughnessTexture, "Material Roughness Texture");
        roughnessTextureView = TextureView::create(roughnessTexture);
    }

    if (heightTexturePath) {
        auto heightTexture = Texture2D::create(heightTexturePath, VK_FORMAT_R8_UNORM, MipmapData::Options::Generate);
        BZ_SET_TEXTURE_DEBUG_NAME(heightTexture, "Material Height Texture");
        heightTextureView = TextureView::create(heightTexture);
    }

    if (aoTexturePath) {
        auto aoTexture = Texture2D::create(aoTexturePath, VK_FORMAT_R8_UNORM, MipmapData::Options::Generate);
        BZ_SET_TEXTURE_DEBUG_NAME(aoTexture, "Material AO Texture");
        aoTextureView = TextureView::create(aoTexture);
    }

    init();
}

void Material::init() {
    const Ref<Sampler> &sampler =
        anisotropicSampler ? Renderer::getDefaultAnisotropicSampler() : Renderer::getDefaultSampler();

    descriptorSet = &Renderer::createMaterialDescriptorSet();
    descriptorSet->setCombinedTextureSampler(albedoTextureView, sampler, 1);

    if (normalTextureView)
        descriptorSet->setCombinedTextureSampler(normalTextureView, sampler, 2);
    else
        descriptorSet->setCombinedTextureSampler(albedoTextureView, sampler, 2);

    if (metallicTextureView)
        descriptorSet->setCombinedTextureSampler(metallicTextureView, sampler, 3);
    else
        descriptorSet->setCombinedTextureSampler(albedoTextureView, sampler, 3);

    if (roughnessTextureView)
        descriptorSet->setCombinedTextureSampler(roughnessTextureView, sampler, 4);
    else
        descriptorSet->setCombinedTextureSampler(albedoTextureView, sampler, 4);

    if (heightTextureView)
        descriptorSet->setCombinedTextureSampler(heightTextureView, sampler, 5);
    else
        descriptorSet->setCombinedTextureSampler(albedoTextureView, sampler, 5);

    if (aoTextureView)
        descriptorSet->setCombinedTextureSampler(aoTextureView, sampler, 6);
    else
        descriptorSet->setCombinedTextureSampler(albedoTextureView, sampler, 6);
}

bool Material::operator==(const Material &other) const {
    return albedoTextureView == other.albedoTextureView && normalTextureView == other.normalTextureView &&
           metallicTextureView == other.metallicTextureView && roughnessTextureView == other.roughnessTextureView &&
           heightTextureView == other.heightTextureView && aoTextureView == other.aoTextureView &&
           metallic == other.metallic && roughness == other.roughness &&
           parallaxOcclusionScale == other.parallaxOcclusionScale && uvScale == other.uvScale &&
           anisotropicSampler == other.anisotropicSampler;
}
}