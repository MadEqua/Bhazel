#include "bzpch.h"

#include "AABB.h"


namespace BZ {

AABB::AABB() : IBoundingVolume(BoundingVolumeType::AABB) {
    empty();
}

AABB::AABB(const AABB &other) : IBoundingVolume(BoundingVolumeType::AABB) {
    min = other.min;
    max = other.max;
}

AABB::AABB(const glm::vec3 *points, uint32 count) : IBoundingVolume(BoundingVolumeType::AABB) {
    empty();

    for (uint32 i = 0; i < count; ++i) {
        enclose(points[i]);
    }
}

AABB::AABB(const AABB &other, const glm::mat4 &transform) : AABB(other) {
    this->transform(transform);
}

AABB::AABB(const glm::vec3 &center, const glm::vec3 &dimensions) : IBoundingVolume(BoundingVolumeType::AABB) {
    min = center - dimensions * 0.5f;
    max = center + dimensions * 0.5f;
}

void AABB::empty() {
    min[0] = min[1] = min[2] = std::numeric_limits<float>::max();
    max[0] = max[1] = max[2] = std::numeric_limits<float>::min();
}

std::unique_ptr<IBoundingVolume> AABB::clone() const {
    return std::make_unique<AABB>(*this);
}

void AABB::enclose(const glm::vec3 &point) {
    if (min[0] > point.x)
        min[0] = point.x;
    if (max[0] < point.x)
        max[0] = point.x;

    if (min[1] > point.y)
        min[1] = point.y;
    if (max[1] < point.y)
        max[1] = point.y;

    if (min[2] > point.z)
        min[2] = point.z;
    if (max[2] < point.z)
        max[2] = point.z;
}

/*
 * We could just transform all the 8 corners and enclose.
 * Instead, for efficiency, we set the min and max to translation position.
 * Then we try to find which of the corner points (min or max) will have the smallest/largest value after the
 * transformation. We do that by minimizing/maximizing each of the coeficients of the matrix multiplication:
 *
 * x' = m00 * x + m01 * y + m02 * z
 * y' = m10 * x + m11 * y + m12 * z
 * z' = m20 * x + m21 * y + m22 * z
 *
 * Ex for the computing the new min: if m00 > 0 then the minimum x' is achieved by replacing x for min.x.
 * Naturally the new maximum is achieved by replacing x for max.x.
 * If m00 < 0 then the minimum x' is achieved by replacing x for max.x and the new maximum is achieved by replacing x
 * for min.x.
 *
 * Repeat for y' and z' to have the new min and max.
 */
void AABB::transform(const glm::mat4 &transform) {
    AABB copy(*this);

    glm::vec3 translation = transform[3];
    copy.min[0] = copy.max[0] = translation.x;
    copy.min[1] = copy.max[1] = translation.y;
    copy.min[2] = copy.max[2] = translation.z;

    // x'
    if (transform[0][0] > 0.0f) {
        copy.min[0] += transform[0][0] * min[0];
        copy.max[0] += transform[0][0] * max[0];
    }
    else {
        copy.min[0] += transform[0][0] * max[0];
        copy.max[0] += transform[0][0] * min[0];
    }

    if (transform[1][0] > 0.0f) {
        copy.min[0] += transform[1][0] * min[1];
        copy.max[0] += transform[1][0] * max[1];
    }
    else {
        copy.min[0] += transform[1][0] * max[1];
        copy.max[0] += transform[1][0] * min[1];
    }

    if (transform[2][0] > 0.0f) {
        copy.min[0] += transform[2][0] * min[2];
        copy.max[0] += transform[2][0] * max[2];
    }
    else {
        copy.min[0] += transform[2][0] * max[2];
        copy.max[0] += transform[2][0] * min[2];
    }

    // y'
    if (transform[0][1] > 0.0f) {
        copy.min[1] += transform[0][1] * min[0];
        copy.max[1] += transform[0][1] * max[0];
    }
    else {
        copy.min[1] += transform[0][1] * max[0];
        copy.max[1] += transform[0][1] * min[0];
    }

    if (transform[1][1] > 0.0f) {
        copy.min[1] += transform[1][1] * min[1];
        copy.max[1] += transform[1][1] * max[1];
    }
    else {
        copy.min[1] += transform[1][1] * max[1];
        copy.max[1] += transform[1][1] * min[1];
    }

    if (transform[2][1] > 0.0f) {
        copy.min[1] += transform[2][1] * min[2];
        copy.max[1] += transform[2][1] * max[2];
    }
    else {
        copy.min[1] += transform[2][1] * max[2];
        copy.max[1] += transform[2][1] * min[2];
    }

    // z'
    if (transform[0][2] > 0.0f) {
        copy.min[2] += transform[0][2] * min[0];
        copy.max[2] += transform[0][2] * max[0];
    }
    else {
        copy.min[2] += transform[0][2] * max[0];
        copy.max[2] += transform[0][2] * min[0];
    }

    if (transform[1][2] > 0.0f) {
        copy.min[2] += transform[1][2] * min[1];
        copy.max[2] += transform[1][2] * max[1];
    }
    else {
        copy.min[2] += transform[1][2] * max[1];
        copy.max[2] += transform[1][2] * min[1];
    }

    if (transform[2][2] > 0.0f) {
        copy.min[2] += transform[2][2] * min[2];
        copy.max[2] += transform[2][2] * max[2];
    }
    else {
        copy.min[2] += transform[2][2] * max[2];
        copy.max[2] += transform[2][2] * min[2];
    }

    // The naive way
    /*glm::vec3 dimen = getDimensions();

    glm::vec3 a = glm::vec3(min[0], min[1], min[2]);
    glm::vec3 b = glm::vec3(min[0], min[1] + dimen.y, min[2]);
    glm::vec3 c = glm::vec3(min[0], min[1], min[2] + dimen.z);
    glm::vec3 d = glm::vec3(min[0], min[1] + dimen.y, min[2] + dimen.z);

    glm::vec3 e = glm::vec3(max[0], max[1], max[2]);
    glm::vec3 f = glm::vec3(max[0], max[1] - dimen.y, max[2]);
    glm::vec3 g = glm::vec3(max[0], max[1], max[2] - dimen.z);
    glm::vec3 h = glm::vec3(max[0], max[1] - dimen.y, max[2] - dimen.z);

    copy.enclose(transform * glm::vec4(a, 1.0f));
    copy.enclose(transform * glm::vec4(b, 1.0f));
    copy.enclose(transform * glm::vec4(c, 1.0f));
    copy.enclose(transform * glm::vec4(d, 1.0f));
    copy.enclose(transform * glm::vec4(e, 1.0f));
    copy.enclose(transform * glm::vec4(f, 1.0f));
    copy.enclose(transform * glm::vec4(g, 1.0f));
    copy.enclose(transform * glm::vec4(h, 1.0f));*/

    *this = copy;
}
}