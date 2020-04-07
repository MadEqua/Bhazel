#pragma once

#include "Mesh.h"
#include "Camera.h"
#include "Transform.h"
#include "Graphics/DescriptorSet.h"
#include "Graphics/Texture.h"


namespace BZ {

    struct DirectionalLight {
        glm::vec3 direction;
        glm::vec3 color;
        float intensity;
    };

    class Entity {
    public:
        Entity(Mesh &mesh, Transform &transform);

        Mesh mesh;
        Transform transform;
    };

    struct SkyBox {
        Mesh mesh;
        Ref<TextureView> irradianceMapView;
    };

    class Scene {
    public:
        Scene();
        Scene(Camera &camera);

        void addEntity(Mesh &mesh, Transform &transform);
        void addDirectionalLight(DirectionalLight &light);
        void enableSkyBox(const char *albedoBasePath, const char *albedoFileNames[6], const char *irradianceMapBasePath, const char *irradianceMapFileNames[6]);
        void setCamera(Camera &camera);

        std::vector<Entity>& getEntities() { return entities; }
        const std::vector<Entity>& getEntities() const { return entities; }

        std::vector<DirectionalLight>& getDirectionalLights() { return lights; }
        const std::vector<DirectionalLight>& getDirectionalLights() const { return lights; }

        bool hasSkyBox() const { return skyBox.mesh.isValid(); }
        const SkyBox& getSkyBox() const { return skyBox; }

        Camera& getCamera() { return *camera; }
        const Camera& getCamera() const { return *camera; }

        const Ref<DescriptorSet>& getDescriptorSet() const { return descriptorSet; }

    private:
        std::vector<Entity> entities;
        std::vector<DirectionalLight> lights;
        Camera *camera = nullptr;
        SkyBox skyBox;

        Ref<DescriptorSet> descriptorSet;
    };
}