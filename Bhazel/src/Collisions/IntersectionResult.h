#pragma once


namespace BZ {

struct IntersectionResult {
    IntersectionResult() : penetration(0.0f), intersects(false) {}

    glm::vec3 penetration; // from BV1 to BV2 (intersects() functions argument order).
    bool intersects;
};
}