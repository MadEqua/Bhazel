#include "bzpch.h"

#include "System.h"


namespace BZ {

void System::addEntity(EntityHandle entity) {
    entities.insert(entity);
}
void System::removeEntity(EntityHandle entity) {
    entities.erase(entity);
}

void SystemManager::entityDestroyed(EntityHandle entity) {
    for (auto const &pair : typeHashToSystems) {
        auto const &system = pair.second;
        system->removeEntity(entity);
    }
}

void SystemManager::entitySignatureChanged(EntityHandle entity, Signature entitySignature) {
    for (auto const &pair : typeHashToSystems) {
        auto const &hash = pair.first;
        auto const &system = pair.second;
        auto const &systemSignature = typeHashToSignatures[hash];

        // Entity signature matches system signature.
        if ((entitySignature & systemSignature) == systemSignature) {
            system->addEntity(entity);
        }
        else {
            system->removeEntity(entity);
        }
    }
}

}