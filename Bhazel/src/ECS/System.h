#pragma once

#include "EcsConstants.h"


namespace BZ {

class System {
  public:
    void addEntity(EntityHandle entity);
    void removeEntity(EntityHandle entity);

  private:
    // Entities that match this System's signature.
    std::set<EntityHandle> entities;
};


/*-------------------------------------------------------------------------------------------*/
// Maintains the registered systems and their signatures.
class SystemManager {
  public:
    template<typename T>
    std::unique_ptr<T> registerSystem(Signature signature) {
        size_t *typeHash = typeid(T).hash_code();
        BZ_ASSERT_CORE(typeHashToSystems.find(typeHash) == typeHashToSystems.end(), "System already registered!");

        auto system = std::make_unique<T>();
        typeHashToSystems[typeHash] = system;
        typeHashToSignatures[typeName] = signature;
        return system;
    }

    // template<typename T>
    // void setSignature(Signature signature) {
    //    size_t *typeHash = typeid(T).hash_code();
    //    BZ_ASSERT_CORE(typeHashToSystems.find(typeHash) != typeHashToSystems.end(),
    //                   "System not registered before use!");
    //
    //    typeHashToSignatures[typeName] = signature;
    //}

    void entityDestroyed(EntityHandle entity);
    void entitySignatureChanged(EntityHandle entity, Signature entitySignature);

  private:
    std::unordered_map<size_t, Signature> typeHashToSignatures;
    std::unordered_map<size_t, std::unique_ptr<System>> typeHashToSystems;
};
}