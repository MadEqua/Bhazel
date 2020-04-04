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

    void Scene::enableSkyBox(const char *cubeTextureBasePath, const char *CubeTextureFileNames[6]) {
        auto textureCubeRef = BZ::TextureCube::create(cubeTextureBasePath, CubeTextureFileNames, TextureFormat::R8G8B8A8_SRGB, true);
        skyBox.mesh = Mesh::createUnitCubeInsides(Material(textureCubeRef));
    }

    void Scene::setCamera(Camera &camera) {
        this->camera = &camera;
    }
}