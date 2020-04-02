#include "bzpch.h"

#include "Scene.h"


namespace BZ {

    Scene::Scene(Camera &camera) : 
        camera(&camera) {
    }

    void Scene::addEntity(Mesh &mesh, Transform &transform) {
        entities.push_back({ mesh, transform });
    }

    void Scene::addDirectionalLight(DirectionalLight &light) {
        lights.push_back(light);
    }

    void Scene::enableSkyBox(const char *cubeTextureBasePath, const char *CubeTextureFileNames[6], TextureFormat format) {
        auto textureCubeRef = BZ::TextureCube::create(cubeTextureBasePath, CubeTextureFileNames, format, true);
        skyBox.mesh = Mesh::createUnitCubeInsides(Material(textureCubeRef));
    }

    void Scene::setCamera(Camera &camera) {
        this->camera = &camera;
    }
}