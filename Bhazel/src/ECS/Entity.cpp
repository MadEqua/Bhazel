#include "bzpch.h"

#include "Entity.h"


namespace BZ {

EntityManager::EntityManager() {
    for (EntityHandle entity = 0; entity < MAX_ENTITIES; ++entity) {
        availableEntities.push(entity);
    }
}

EntityHandle EntityManager::createEntity() {
    BZ_ASSERT_CORE(livingEntityCount < MAX_ENTITIES, "Too many entities in existence!");

    EntityHandle handle = availableEntities.front();
    availableEntities.pop();
    livingEntityCount++;

    return handle;
}

void EntityManager::destroyEntity(EntityHandle entity) {
    BZ_ASSERT_CORE(entity < MAX_ENTITIES, "Entity out of range!");

    signatures[entity].reset();

    availableEntities.push(entity);
    livingEntityCount--;
}

void EntityManager::setSignature(EntityHandle entity, Signature signature) {
    BZ_ASSERT_CORE(entity < MAX_ENTITIES, "Entity out of range!");

    signatures[entity] = signature;
}

Signature EntityManager::getSignature(EntityHandle entity) {
    BZ_ASSERT_CORE(entity < MAX_ENTITIES, "Entity out of range!");

    return signatures[entity];
};
}