#pragma once

#include "EcsConstants.h"


namespace BZ {

// An interface is used so that we can create lists of ComponentContainers.
class IComponentContainer {
  public:
    virtual ~IComponentContainer() = default;
    virtual void entityDestroyed(EntityHandle entity) = 0; // TODO: is this needed?
};


/*-------------------------------------------------------------------------------------------*/
// Holds all components of a given type on a single place, for cache efficiency.
template<typename ComponentT>
class ComponentContainer : public IComponentContainer {
  public:
    // Add a Component to an Entity.
    ComponentHandle add(EntityHandle ent, const ComponentT &comp) {
        BZ_ASSERT_CORE(size < MAX_COMPONENT_COUNT, "Reached the maximum component count!");

        ComponentHandle compHandle = size++;
        components[compHandle] = comp;
        entityToComponent[ent] = compHandle;
        componentToEntity[compHandle] = ent;
        return compHandle;
    }

    // Remove a Component from an Entity.
    void remove(EntityHandle ent) {
        if (hasComponent(ent)) {
            ComponentHandle compHandle = entityToComponent[ent];
            ComponentHandle lastComponentHandle = --size;
            components[compHandle] = components[lastComponentHandle];

            entityToComponent.erase(ent);
            // componentToEntity.erase(compHandle);

            EntityHandle movedEntity = componentToEntity[lastComponentHandle];
            entityToComponent[movedEntity] = compHandle;
            componentToEntity[compHandle] = movedEntity;
        }
    }

    ComponentT &get(EntityHandle entity) {
        BZ_ASSERT_CORE(hasComponent(entity), "Non-existent component!");

        return componentArray[entityToComponent[entity]];
    }

    void entityDestroyed(EntityHandle entity) override { remove(entity); }

    // TODO
    // std::array<ComponentT, MAX_COMPONENT_COUNT>::iterator begin() { return components.begin(); }
    // std::array<ComponentT, MAX_COMPONENT_COUNT>::iterator end() { return components.end(); }
    // std::array<ComponentT, MAX_COMPONENT_COUNT>::const_iterator begin() const { return components.cbegin(); }
    // std::array<ComponentT, MAX_COMPONENT_COUNT>::const_iterator end() const { return components.cend(); }

  private:
    // Linear component array for iteration.
    std::array<ComponentT, MAX_COMPONENT_COUNT> components;

    int size = 0;

    // Access a Component given an Entity. ComponentHandles are indices to ComponentArray.
    std::unordered_map<EntityHandle, ComponentHandle> entityToComponent;

    // Inverse of the above.
    std::unordered_map<ComponentHandle, EntityHandle> componentToEntity;
};


/*-------------------------------------------------------------------------------------------*/
// Manages all the ComponentContainers and provide a single interface into them.
class ComponentManager {
  public:
    template<typename T>
    void registerComponent() {
        size_t *typeHash = typeid(T).hash_code();
        BZ_ASSERT_CORE(typeHashToComponentTypeIds.find(typeHash) == typeHashToComponentTypeIds.end(),
                       "Component already registered!");

        typeHashToComponentTypeIds[typeHash] = nextComponentTypeId++;
        typeHashToComponentContainers[typeHash] = std::make_unique<ComponentContainer<T>>();
    }

    template<typename T>
    ComponentTypeId getComponentTypeId() {
        size_t *typeHash = typeid(T).hash_code();
        BZ_ASSERT_CORE(typeHashToComponentTypeIds.find(typeHash) != typeHashToComponentTypeIds.end(),
                       "Component not registered before use!");

        return typeHashToComponentTypeIds[typeHash];
    }

    template<typename T>
    void addComponent(EntityHandle entity, T component) {
        getComponentContainer<T>()->insert(entity, component);
    }

    template<typename T>
    void removeComponent(EntityHandle entity) {
        getComponentContainer<T>()->remove(entity);
    }

    template<typename T>
    T &getComponent(EntityHandle entity) {
        return getComponentContainer<T>()->get(entity);
    }

    void entityDestroyed(EntityHandle entity) {
        for (auto const &pair : typeHashToComponentContainers) {
            auto const &container = pair.second;
            container->entityDestroyed(entity);
        }
    }

  private:
    std::unordered_map<size_t, ComponentTypeId> typeHashToComponentTypeIds;
    std::unordered_map<size_t, std::unique_ptr<IComponentContainer>> typeHashToComponentContainers;

    // The component type to be assigned to the next registered component.
    ComponentTypeId nextComponentTypeId = 0;

    // Convenience function to get the pointer to the ComponentContainer of type T.
    template<typename T>
    std::unique_ptr<ComponentContainer<T>> getComponentContainer() {
        size_t *typeHash = typeid(T).hash_code();
        BZ_ASSERT_CORE(componentTypes.find(typeHash) != componentTypes.end(), "Component not registered before use!");
        return std::static_pointer_cast<ComponentContainer<T>>(componentContainers[typeHash]);
    }
};
}
