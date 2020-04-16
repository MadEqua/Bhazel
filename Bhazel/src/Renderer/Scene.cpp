#include "bzpch.h"

#include "Scene.h"
#include "Renderer.h"
#include "Graphics/RenderPass.h"


namespace BZ {

    DirectionalLight::DirectionalLight() :
        camera(-20, 20, -20, 20, 0.1f, 100.0f) {
        shadowMapFramebuffer = Renderer::createShadowMapFramebuffer();
    }

    void DirectionalLight::setDirection(const glm::vec3 &direction) {
        this->direction = direction;
        camera.getTransform().setTranslation(-direction);
        camera.getTransform().lookAt(glm::vec3(0.0f));
    }


    Entity::Entity(Mesh &mesh, Transform &transform) :
        mesh(mesh), transform(transform) {
    }


    Scene::Scene() {
        descriptorSet = Renderer::createSceneDescriptorSet();
    }

    Scene::Scene(Camera &camera) :
        camera(&camera) {
        descriptorSet = Renderer::createSceneDescriptorSet();
    }

    void Scene::addEntity(Mesh &mesh, Transform &transform) {
        entities.push_back({ mesh, transform });
    }

    void Scene::addDirectionalLight(DirectionalLight &light) {
        lights.push_back(light);
    }

    void Scene::enableSkyBox(const char *albedoBasePath, const char *albedoFileNames[6],
                             const char *irradianceMapBasePath, const char *irradianceMapFileNames[6],
                             const char *radianceMapBasePath, const char *radianceMapFileNames[6], uint32 radianceMipmapCount) {
        
        auto albedoTexRef = TextureCube::create(albedoBasePath, albedoFileNames, TextureFormat::R8G8B8A8_SRGB, MipmapData::Options::Generate);
        skyBox.mesh = Mesh::createUnitCubeInsides(Material(albedoTexRef));

        auto irradianceMapTexRef = TextureCube::create(irradianceMapBasePath, irradianceMapFileNames, TextureFormat::R8G8B8A8_SRGB, MipmapData::Options::Generate);
        skyBox.irradianceMapView = TextureView::create(irradianceMapTexRef);

        auto radianceMapTexRef = TextureCube::create(radianceMapBasePath, radianceMapFileNames, TextureFormat::R8G8B8A8_SRGB, { MipmapData::Options::Load, radianceMipmapCount });
        skyBox.radianceMapView = TextureView::create(radianceMapTexRef);

        descriptorSet->setCombinedTextureSampler(skyBox.irradianceMapView, Renderer::getDefaultSampler(), 1);
        descriptorSet->setCombinedTextureSampler(skyBox.radianceMapView, Renderer::getDefaultSampler(), 2);
    }

    void Scene::setCamera(Camera &camera) {
        this->camera = &camera;
    }
}