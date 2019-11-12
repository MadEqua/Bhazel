#pragma once

#include <glm/glm.hpp>

//TODO

namespace BZ {

    union ClearValues {
        glm::vec4 floating;
        glm::ivec4 integer;
    };
}