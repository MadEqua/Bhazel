#pragma once

#include "EcsConstants.h"
#include "Entity.h"
#include "Component.h"
#include "System.h"


namespace BZ {


class EcsRoot {
  public:
    // Entity methods.
    EntityHandle createEntity();
    void destroyEntity(EntityHandle entity);

    // Component methods.
    template<typename T>
    void registerComponent() {
        componentManager.registerComponent<T>();
    }

    template<typename T>
    void addComponent(EntityHandle entity, T component) {
        componentManager.addComponent<T>(entity, component);

        auto signature = entityManager.getSignature(entity);
        signature.set(componentManager.getComponentType<T>(), true);
        entityManager.setSignature(entity, signature);

        systemManager.entitySignatureChanged(entity, signature);
    }

    template<typename T>
    void removeComponent(EntityHandle entity) {
        componentManager.removeComponent<T>(entity);

        auto signature = entityManager.getSignature(entity);
        signature.set(componentManager.getComponentType<T>(), false);
        entityManager.setSignature(entity, signature);

        systemManager.entitySignatureChanged(entity, signature);
    }

    template<typename T>
    T &getComponent(EntityHandle entity) {
        return componentManager.getComponent<T>(entity);
    }

    template<typename T>
    ComponentTypeId getComponentTypeId() {
        return componentManager.getComponentTypeId<T>();
    }

    // System methods.
    template<typename T>
    std::unique_ptr<T> registerSystem(Signature signature) {
        return systemManager.registerSystem<T>(signature);
    }

    //template<typename T>
    //void setSystemSignature(Signature signature) {
    //    systemManager.setSignature<T>(signature);
    //}

  private:
    EntityManager entityManager;
    ComponentManager componentManager;
    SystemManager systemManager;
};
}