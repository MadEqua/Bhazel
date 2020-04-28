#pragma once

#include "Mesh.h"
#include "Camera.h"
#include "Transform.h"

#include "Graphics/DescriptorSet.h"
#include "Graphics/Framebuffer.h"


namespace BZ {

    class DirectionalLight {
    public:
        DirectionalLight();

        glm::vec3 color;
        float intensity;

        const glm::vec3& getDirection() const { return direction; }
        void setDirection(const glm::vec3 &direction);

        //TODO: does this belong here?
        std::array<Ref<Framebuffer>, SHADOW_MAPPING_CASCADE_COUNT> shadowMapFramebuffers;

    private:
        glm::vec3 direction;
    };

    class Entity {
    public:
        Entity(Mesh &mesh, Transform &transform, bool castShadow);
        Entity(Mesh &mesh, Transform &transform, Material &overrideMaterial, bool castShadow);

        Mesh mesh;
        Transform transform;
        bool castShadow;

        //If present, will override the Mesh Material
        Material overrideMaterial;
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

        void addEntity(Mesh &mesh, Transform &transform, bool castShadow = true);
        void addEntity(Mesh &mesh, Transform &transform, Material &overrideMaterial, bool castShadow = true);
        void addDirectionalLight(DirectionalLight &light);
        void enableSkyBox(const char *albedoBasePath, const char *albedoFileNames[6],
                          const char *irradianceMapBasePath, const char *irradianceMapFileNames[6],
                          const char *radianceMapBasePath, const char *radianceMapFileNames[6], uint32 radianceMipmapCount);
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