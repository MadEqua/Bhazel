#pragma once

#include "Mesh.h"
#include "Camera.h"
#include "Transform.h"


namespace BZ {

    struct DirectionalLight {
        glm::vec3 direction;
        glm::vec3 color;
        float intensity;
    };

    struct Entity {
        Mesh mesh;
        Transform transform;
    };

    struct SkyBox {
        Mesh mesh;
    };

    class Scene {
    public:
        Scene() = default;
        Scene(Camera &camera);

        void addEntity(Mesh &mesh, Transform &transform);
        void addDirectionalLight(DirectionalLight &light);
        void enableSkyBox(const char *cubeTextureBasePath, const char *CubeTextureFileNames[6], TextureFormat format);
        void setCamera(Camera &camera);

        std::vector<Entity>& getEntities() { return entities; }
        const std::vector<Entity>& getEntities() const { return entities; }

        std::vector<DirectionalLight>& getDirectionalLights() { return lights; }
        const std::vector<DirectionalLight>& getDirectionalLights() const { return lights; }

        bool hasSkyBox() const { return skyBox.mesh.isValid(); }
        const SkyBox& getSkyBox() const { return skyBox; }

        Camera& getCamera() { return *camera; }
        const Camera& getCamera() const { return *camera; }

    private:
        std::vector<Entity> entities;
        std::vector<DirectionalLight> lights;
        Camera *camera = nullptr;
        SkyBox skyBox;
    };
}