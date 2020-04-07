#include "bzpch.h"

#include "Scene.h"
#include "Renderer.h"


namespace BZ {

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

    void Scene::enableSkyBox(const char *albedoBasePath, const char *albedoFileNames[6], const char *irradianceMapBasePath, const char *irradianceMapFileNames[6]) {
        auto albedoTexRef = TextureCube::create(albedoBasePath, albedoFileNames, TextureFormat::R8G8B8A8_SRGB, true);
        skyBox.mesh = Mesh::createUnitCubeInsides(Material(albedoTexRef));

        auto irradianceMapTexRef = TextureCube::create(irradianceMapBasePath, irradianceMapFileNames, TextureFormat::R8G8B8A8_SRGB, true);
        skyBox.irradianceMapView = TextureView::create(irradianceMapTexRef);

        descriptorSet->setCombinedTextureSampler(skyBox.irradianceMapView, Renderer::getDefaultSampler(), 1);
    }

    void Scene::setCamera(Camera &camera) {
        this->camera = &camera;
    }
}