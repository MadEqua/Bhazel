#pragma once

#include "EcsConstants.h"


namespace BZ {

class EntityManager {
  public:
    EntityManager();

    EntityHandle createEntity();
    void destroyEntity(EntityHandle entity);

    void setSignature(EntityHandle entity, Signature signature);
    Signature getSignature(EntityHandle entity);

  private:
    // Queue of unused entity handles.
    std::queue<EntityHandle> availableEntities;

    // Array of signatures where the index corresponds to the entity handle.
    std::array<Signature, MAX_ENTITIES> signatures;

    uint32 livingEntityCount;
};
}