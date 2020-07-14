#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>


namespace BZ {

class TextureView;
class Texture2D;
class TextureCube;
class DescriptorSet;

class Material {
  public:
    Material() = default;

    Material(const Ref<Texture2D> &albedoTexture, const Ref<Texture2D> &normalTexture = MakeRefNull<Texture2D>(),
             const Ref<Texture2D> &metallicTexture = MakeRefNull<Texture2D>(),
             const Ref<Texture2D> &roughnessTexture = MakeRefNull<Texture2D>(),
             const Ref<Texture2D> &heightTexture = MakeRefNull<Texture2D>(),
             const Ref<Texture2D> &aoTexture = MakeRefNull<Texture2D>(), bool useAnisotropicSampler = false);

    explicit Material(Ref<TextureCube> &albedoTexture);

    Material(const char *albedoTexturePath, const char *normalTexturePath = nullptr,
             const char *metallicTexturePath = nullptr, const char *roughnessTexturePath = nullptr,
             const char *heightTexturePath = nullptr, const char *aoTexturePath = nullptr,
             bool useAnisotropicSampler = false);

    bool isValid() const { return static_cast<bool>(descriptorSet); }
    const DescriptorSet &getDescriptorSet() const { return *descriptorSet; }

    const Ref<TextureView> &getAlbedoTextureView() const { return albedoTextureView; }
    const Ref<TextureView> &getNormalTextureView() const { return normalTextureView; }
    const Ref<TextureView> &getMetallicTextureView() const { return metallicTextureView; }
    const Ref<TextureView> &getRoughnessTextureView() const { return roughnessTextureView; }
    const Ref<TextureView> &getHeightTextureView() const { return heightTextureView; }
    const Ref<TextureView> &getAOTextureView() const { return aoTextureView; }

    bool hasNormalTexture() const { return static_cast<bool>(normalTextureView); }
    bool hasMetallicTexture() const { return static_cast<bool>(metallicTextureView); }
    bool hasRoughnessTexture() const { return static_cast<bool>(roughnessTextureView); }
    bool hasHeightTexture() const { return static_cast<bool>(heightTextureView); }
    bool hasAOTexture() const { return static_cast<bool>(aoTextureView); }

    float getMetallic() const { return metallic; }
    void setMetallic(float met) { metallic = met; }

    float getRoughness() const { return roughness; }
    void setRoughness(float rough) { roughness = rough; }

    float getParallaxOcclusionScale() const { return parallaxOcclusionScale; }
    void setParallaxOcclusionScale(float scale) { parallaxOcclusionScale = scale; }

    bool useAnisotropicSampler() const { return anisotropicSampler; }

    const glm::vec2 &getUvScale() const { return uvScale; }
    void setUvScale(float u, float v) {
        uvScale.x = u;
        uvScale.y = v;
    }
    void setUvScale(const glm::vec2 &mult) { uvScale = mult; }

    bool operator==(const Material &other) const;

  private:
    bool anisotropicSampler = false;

    Ref<TextureView> albedoTextureView;
    Ref<TextureView> normalTextureView;
    Ref<TextureView> metallicTextureView;
    Ref<TextureView> roughnessTextureView;
    Ref<TextureView> heightTextureView;
    Ref<TextureView> aoTextureView;

    DescriptorSet *descriptorSet = nullptr;

    // Valid if no respective textures are present.
    float metallic = 1.0f;
    float roughness = 0.0f;

    float parallaxOcclusionScale = 0.0f;
    glm::vec2 uvScale = { 1.0f, 1.0f };

    void init();
};
}

template<>
struct std::hash<BZ::Material> {
    size_t operator()(const BZ::Material &mat) const {
        return (std::hash<BZ::Ref<BZ::TextureView>>()(mat.getAlbedoTextureView())) ^
               (std::hash<BZ::Ref<BZ::TextureView>>()(mat.getNormalTextureView())) ^
               (std::hash<BZ::Ref<BZ::TextureView>>()(mat.getMetallicTextureView())) ^
               (std::hash<BZ::Ref<BZ::TextureView>>()(mat.getRoughnessTextureView())) ^
               (std::hash<BZ::Ref<BZ::TextureView>>()(mat.getHeightTextureView())) ^
               (std::hash<BZ::Ref<BZ::TextureView>>()(mat.getAOTextureView())) ^
               (std::hash<float>()(mat.getMetallic())) ^ (std::hash<float>()(mat.getRoughness())) ^
               (std::hash<float>()(mat.getParallaxOcclusionScale())) ^ (std::hash<glm::vec2>()(mat.getUvScale())) ^
               (std::hash<bool>()(mat.useAnisotropicSampler()));
    }
};
