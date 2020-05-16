#include "bzpch.h"

#include "Scene.h"
#include "Renderer.h"

#include "Graphics/RenderPass.h"
#include "Graphics/DescriptorSet.h"
#include "Graphics/Framebuffer.h"

#include "Collisions/AABB.h"


namespace BZ {

    DirectionalLight::DirectionalLight() {
        shadowMapFramebuffer = Renderer::createShadowMapFramebuffer();
    }

    void DirectionalLight::setDirection(const glm::vec3 &direction) {
        this->direction = glm::normalize(direction);
    }


    Entity::Entity(Mesh &mesh, Transform &transform, bool castShadow) :
        mesh(mesh), transform(transform), castShadow(castShadow) {
    }

    Entity::Entity(Mesh &mesh, Transform &transform, Material &overrideMaterial, bool castShadow):
        mesh(mesh), transform(transform), overrideMaterial(overrideMaterial), castShadow(castShadow) {
    }


    Scene::Scene() {
        descriptorSet = &Renderer::createSceneDescriptorSet();
    }

    Scene::Scene(Camera &camera) :
        camera(&camera) {
        descriptorSet = &Renderer::createSceneDescriptorSet();
    }

    void Scene::addEntity(Mesh &mesh, Transform &transform, bool castShadow) {
        BZ_ASSERT_CORE(entities.size() < MAX_ENTITIES_PER_SCENE, "Reached the maximum ammount of Entities!");
        entities.push_back({ mesh, transform, castShadow });
    }

    void Scene::addEntity(Mesh &mesh, Transform &transform, Material &overrideMaterial, bool castShadow) {
        BZ_ASSERT_CORE(entities.size() < MAX_ENTITIES_PER_SCENE, "Reached the maximum ammount of Entities!");
        entities.push_back({ mesh, transform, overrideMaterial, castShadow });
    }

    void Scene::addDirectionalLight(DirectionalLight &light) {
        BZ_ASSERT_CORE(lights.size() < MAX_DIR_LIGHTS_PER_SCENE, "Reached the maximum ammount of Directional Lights!");
        lights.push_back(light);

        //TODO: this is ugly and bad.
        std::array<Ref<TextureView>, MAX_DIR_LIGHTS_PER_SCENE> shadowMaps;
        for (uint32 i = 0; i < MAX_DIR_LIGHTS_PER_SCENE; ++i) {
            if (i < lights.size()) {
                shadowMaps[i] = lights[i].shadowMapFramebuffer->getDepthStencilTextureView();
            }
            else {
                shadowMaps[i] = Renderer::getDummyTextureArrayView();
            }
        }
        descriptorSet->setCombinedTextureSamplers(shadowMaps.data(), MAX_DIR_LIGHTS_PER_SCENE, 0, Renderer::getShadowSampler(), 3);
    }

    void Scene::enableSkyBox(const char *albedoBasePath, const char *albedoFileNames[6],
                             const char *irradianceMapBasePath, const char *irradianceMapFileNames[6],
                             const char *radianceMapBasePath, const char *radianceMapFileNames[6], uint32 radianceMipmapCount) {

        auto albedoTexRef = TextureCube::create(albedoBasePath, albedoFileNames, VK_FORMAT_R32G32B32A32_SFLOAT, MipmapData::Options::Generate);
        skyBox.mesh = Mesh::createUnitCubeInsides(Material(albedoTexRef));

        auto irradianceMapTexRef = TextureCube::create(irradianceMapBasePath, irradianceMapFileNames, VK_FORMAT_R32G32B32A32_SFLOAT, MipmapData::Options::Generate);
        skyBox.irradianceMapView = TextureView::create(irradianceMapTexRef);

        auto radianceMapTexRef = TextureCube::create(radianceMapBasePath, radianceMapFileNames, VK_FORMAT_R32G32B32A32_SFLOAT, { MipmapData::Options::Load, radianceMipmapCount });
        skyBox.radianceMapView = TextureView::create(radianceMapTexRef);

        descriptorSet->setCombinedTextureSampler(skyBox.irradianceMapView, Renderer::getDefaultSampler(), 1);
        descriptorSet->setCombinedTextureSampler(skyBox.radianceMapView, Renderer::getDefaultSampler(), 2);
    }

    void Scene::setCamera(Camera &camera) {
        this->camera = &camera;
    }
}