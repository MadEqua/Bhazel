#include "bzpch.h"

#include "EcsRoot.h"


namespace BZ {

EntityHandle EcsRoot::createEntity() {
    return entityManager.createEntity();
}

void EcsRoot::destroyEntity(EntityHandle entity) {
    entityManager.destroyEntity(entity);

    componentManager.entityDestroyed(entity);
    systemManager.entityDestroyed(entity);
}
}