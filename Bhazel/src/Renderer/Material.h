#pragma once

#include "Graphics/Texture.h"
#include "Graphics/DescriptorSet.h"


namespace BZ {

    class Material {
    public:
        Material() = default;

        Material(Ref<Texture2D> &albedoTexture, Ref<Texture2D> &normalTexture, Ref<Texture2D> &metallicTexture, Ref<Texture2D> &roughnessTextureView, Ref<Texture2D> &heightTextureView);
        Material(Ref<Texture2D> &albedoTexture, Ref<Texture2D> &normalTexture);
        explicit Material(Ref<TextureCube> &albedoTexture);

        Material(const char *albedoTexturePath, const char *normalTexturePath, const char *metallicTexturePath, const char *roughnessTexturePath, const char *heightTexturePath);
        Material(const char *albedoTexturePath, const char *normalTexturePath);

        bool isValid() const { return static_cast<bool>(descriptorSet); }

        const Ref<TextureView>& getAlbedoTextureView() const { return albedoTextureView; }
        const Ref<TextureView>& getNormalTextureView() const { return normalTextureView; }
        const Ref<TextureView>& getMetallicTextureView() const { return metallicTextureView; }
        const Ref<TextureView>& getRoughnessTextureView() const { return roughnessTextureView; }
        const Ref<TextureView>& getHeightTextureView() const { return heightTextureView; }

        const Ref<DescriptorSet>& getDescriptorSet() const { return descriptorSet; }

        float getParallaxOcclusionScale() const { return parallaxOcclusionScale; }
        void setParallaxOcclusionScale(float scale) { parallaxOcclusionScale = scale; }

        const glm::vec2& getUvScale() const { return uvScale; }
        void setUvScale(const glm::vec2& mult) { uvScale = mult; }

        bool operator==(const Material &other) const;

    private:
        Ref<TextureView> albedoTextureView;
        Ref<TextureView> normalTextureView;
        Ref<TextureView> metallicTextureView;
        Ref<TextureView> roughnessTextureView;
        Ref<TextureView> heightTextureView;

        Ref<DescriptorSet> descriptorSet;

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
            std::hash<float>()(mat.getParallaxOcclusionScale());
    }
};
