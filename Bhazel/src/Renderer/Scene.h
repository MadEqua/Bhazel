#pragma once

#include "Mesh.h"
#include "Camera.h"
#include "Transform.h"

#include "Graphics/DescriptorSet.h"
#include "Graphics/Framebuffer.h"


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
        Ref<TextureView> radianceMapView;
    };

    class Scene {
    public:
        Scene();
        Scene(Camera &camera);

        void addEntity(Mesh &mesh, Transform &transform);
        void addDirectionalLight(DirectionalLight &light);
        void enableSkyBox(const char *albedoBasePath, const char *albedoFileNames[6],
                          const char *irradianceMapBasePath, const char *irradianceMapFileNames[6],
                          const char *radianceMapBasePath, const char *radianceMapFileNames[6], uint32 radianceMipmapCount);
        void setCamera(Camera &camera);

        std::vector<Entity>& getEntities() { return entities; }
        const std::vector<Entity>& getEntities() const { return entities; }

        std::vector<DirectionalLight>& getDirectionalLights() { return lights; }
        const std::vector<DirectionalLight>& getDirectionalLights() const { return lights; }

        const std::vector< Ref<Framebuffer>>& getShadowMapFramebuffers() const { return shadowMapFramebuffers; }

        bool hasSkyBox() const { return skyBox.mesh.isValid(); }
        const SkyBox& getSkyBox() const { return skyBox; }

        Camera& getCamera() { return *camera; }
        const Camera& getCamera() const { return *camera; }

        const Ref<DescriptorSet>& getDescriptorSet() const { return descriptorSet; }

    private:
        std::vector<Entity> entities;

        std::vector<DirectionalLight> lights;
        std::vector<Ref<Framebuffer>> shadowMapFramebuffers;

        Camera *camera = nullptr;
        SkyBox skyBox;

        Ref<DescriptorSet> descriptorSet;
    };
}