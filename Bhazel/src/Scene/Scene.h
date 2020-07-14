#pragma once

//#include "Renderer/Components/Camera.h"
//#include "Renderer/Components/Mesh.h"
//#include "Renderer/Components/Transform.h"
#include "Layers/LayerStack.h"

#include <entt/entity/registry.hpp>


namespace BZ {

// class DescriptorSet;
// class Framebuffer;
//
// class DirectionalLight {
//  public:
//    DirectionalLight();
//
//    glm::vec3 color;
//    float intensity;
//
//    const glm::vec3 &getDirection() const { return direction; }
//    void setDirection(const glm::vec3 &direction);
//
//    // TODO: does this belong here?
//    Ref<Framebuffer> shadowMapFramebuffer;
//
//  private:
//    glm::vec3 direction;
//};
//
// class Entity {
//  public:
//    Entity(Mesh &mesh, Transform &transform, bool castShadow);
//    Entity(Mesh &mesh, Transform &transform, Material &overrideMaterial, bool castShadow);
//
//    Mesh mesh;
//    Transform transform;
//    bool castShadow;
//
//    // If present, will override the Mesh Material
//    Material overrideMaterial;
//};
//
// struct SkyBox {
//    Mesh mesh;
//    Ref<TextureView> irradianceMapView;
//    Ref<TextureView> radianceMapView;
//};
//
// class Scene {
//  public:
//    Scene();
//    Scene(Camera &camera);
//
//    void addEntity(Mesh &mesh, Transform &transform, bool castShadow = true);
//    void addEntity(Mesh &mesh, Transform &transform, Material &overrideMaterial, bool castShadow = true);
//    void addDirectionalLight(DirectionalLight &light);
//    void enableSkyBox(const char *albedoBasePath, const char *albedoFileNames[6], const char *irradianceMapBasePath,
//                      const char *irradianceMapFileNames[6], const char *radianceMapBasePath,
//                      const char *radianceMapFileNames[6], uint32 radianceMipmapCount);
//    void setCamera(Camera &camera);
//
//    std::vector<Entity> &getEntities() { return entities; }
//    const std::vector<Entity> &getEntities() const { return entities; }
//
//    std::vector<DirectionalLight> &getDirectionalLights() { return lights; }
//    const std::vector<DirectionalLight> &getDirectionalLights() const { return lights; }
//
//    bool hasSkyBox() const { return skyBox.mesh.isValid(); }
//    const SkyBox &getSkyBox() const { return skyBox; }
//
//    Camera &getCamera() { return *camera; }
//    const Camera &getCamera() const { return *camera; }
//
//    const DescriptorSet &getDescriptorSet() const { return *descriptorSet; }
//
//  private:
//    std::vector<Entity> entities;
//    std::vector<DirectionalLight> lights;
//
//    Camera *camera = nullptr;
//    SkyBox skyBox;
//
//    DescriptorSet *descriptorSet = nullptr;
//};

class Scene {
  public:
    Scene();
    virtual ~Scene();

    // EntityHandle createEntity() { return ecsInstance.createEntity(); }
    // void destroyEntity(EntityHandle entity) { ecsInstance.destroyEntity(entity); }

    virtual void onAttachToEngine() { layerStack.onAttachToEngine(); }
    virtual void onUpdate(const FrameTiming &frameTiming) { layerStack.onUpdate(frameTiming); }
    virtual void onImGuiRender(const FrameTiming &frameTiming) { layerStack.onImGuiRender(frameTiming); }
    virtual void onEvent(Event &e) { layerStack.onEvent(e); }

    void pushLayer(Layer *layer) { layerStack.pushLayer(layer); }
    void pushOverlay(Layer *overlay) { layerStack.pushOverlay(overlay); }

    entt::registry &getEcsInstance() { return ecsInstance; }
    const entt::registry &getEcsInstance() const { return ecsInstance; }

    //entt::entity createEntity();


  protected:
    LayerStack layerStack;

    entt::registry ecsInstance;
};
}