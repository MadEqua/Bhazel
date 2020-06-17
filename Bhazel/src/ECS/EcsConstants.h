#pragma once

namespace BZ {

constexpr uint32 MAX_ENTITIES = 1024;
constexpr uint32 MAX_COMPONENT_COUNT = 32;

using EntityHandle = uint32;
using ComponentHandle = uint32;
using ComponentTypeId = uint8;
using Signature = std::bitset<MAX_COMPONENT_COUNT>;

}
