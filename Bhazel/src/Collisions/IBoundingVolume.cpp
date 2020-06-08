#include "bzpch.h"

#include "IBoundingVolume.h"

#include "AABB.h"
#include "BoundingSphere.h"
#include "CollisionUtils.h"


namespace BZ {

IBoundingVolume::IBoundingVolume(BoundingVolumeType type) : type(type) {
}

void IBoundingVolume::enclose(const IBoundingVolume &other) {
    if (type == BoundingVolumeType::AABB) {
        if (other.getType() == BoundingVolumeType::AABB) {
            CollisionUtils::enclose(static_cast<AABB &>(*this), static_cast<const AABB &>(other));
        }
        else if (other.getType() == BoundingVolumeType::Sphere) {
            CollisionUtils::enclose(static_cast<AABB &>(*this), static_cast<const BoundingSphere &>(other));
        }
    }
    else if (type == BoundingVolumeType::Sphere) {
        if (other.getType() == BoundingVolumeType::AABB) {
            CollisionUtils::enclose(static_cast<BoundingSphere &>(*this), static_cast<const AABB &>(other));
        }
        else if (other.getType() == BoundingVolumeType::Sphere) {
            CollisionUtils::enclose(static_cast<BoundingSphere &>(*this), static_cast<const BoundingSphere &>(other));
        }
    }
}

IntersectionResult IBoundingVolume::intersects(const IBoundingVolume &other) const {
    if (type == BoundingVolumeType::AABB) {
        if (other.getType() == BoundingVolumeType::AABB) {
            return CollisionUtils::intersects(static_cast<const AABB &>(*this), static_cast<const AABB &>(other));
        }
        else if (other.getType() == BoundingVolumeType::Sphere) {
            return CollisionUtils::intersects(static_cast<const AABB &>(*this),
                                              static_cast<const BoundingSphere &>(other));
        }
    }
    else if (type == BoundingVolumeType::Sphere) {
        if (other.getType() == BoundingVolumeType::AABB) {
            return CollisionUtils::intersects(static_cast<const BoundingSphere &>(*this),
                                              static_cast<const AABB &>(other));
        }
        else if (other.getType() == BoundingVolumeType::Sphere) {
            return CollisionUtils::intersects(static_cast<const BoundingSphere &>(*this),
                                              static_cast<const BoundingSphere &>(other));
        }
    }
    return IntersectionResult();
}
}