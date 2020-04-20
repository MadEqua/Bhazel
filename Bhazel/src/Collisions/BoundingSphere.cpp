#include "bzpch.h"

#include "BoundingSphere.h"


namespace BZ {

    BoundingSphere::BoundingSphere() :
        IBoundingVolume(BoundingVolumeType::Sphere) {
        empty();
    }

    BoundingSphere::BoundingSphere(const BoundingSphere &other):
        IBoundingVolume(BoundingVolumeType::Sphere),
        center(other.center),
        radius(other.radius) {
    }

    BoundingSphere::BoundingSphere(const glm::vec3 *points, uint32 count) :
        IBoundingVolume(BoundingVolumeType::Sphere) {
        empty();

        for (uint32 i = 0; i < count; ++i) {
            center += points[i];
        }
        center /= static_cast<float>(count);

        for (uint32 i = 0; i < count; ++i) {
            enclose(points[i]);
        }
    }

    BoundingSphere::BoundingSphere(const BoundingSphere &other, const glm::mat4 &transform) :
        BoundingSphere(other) {
        this->transform(transform);
    }

    BoundingSphere::BoundingSphere(const glm::vec3 &center, float radius) :
        IBoundingVolume(BoundingVolumeType::AABB) {
        this->center = center;
        this->radius = radius;
    }

    std::unique_ptr<IBoundingVolume> BoundingSphere::clone() const {
        return std::make_unique<BoundingSphere>(*this);
    }

    void BoundingSphere::empty() {
        center[0] = center[1] = center[2] = radius = 0.0f;
    }

    void BoundingSphere::enclose(const glm::vec3 &point) {
        float distance = glm::distance(point, center);
        if (distance > radius) {
            radius = distance;
        }
    }

    void BoundingSphere::transform(const glm::mat4 &transform) {
        glm::vec4 newCenter = transform * glm::vec4(center, 1.0f);
        glm::vec4 newOtherPoint = transform * glm::vec4(center + glm::vec3(radius, 0.0, 0.0), 1.0f);
        center = glm::vec3(newCenter);
        radius = glm::distance(center, glm::vec3(newOtherPoint));
    }
}