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

    void Scene::setCamera(Camera &camera) {
        this->camera = &camera;
    }
}