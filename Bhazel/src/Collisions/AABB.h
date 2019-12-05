#pragma once

#include "IBoundingVolume.h"


namespace BZ {

    class AABB : public IBoundingVolume {
    public:
        AABB();
        AABB(const AABB &other);

        //Will enclose all points.
        explicit AABB(const std::vector<glm::vec3> &points);

        //Will enclose the other transformed AABB.
        AABB(const AABB &other, const glm::mat4 &transform);

        AABB(const glm::vec3 &center, const glm::vec3 &dimensions);

        std::unique_ptr<IBoundingVolume> clone() const override;

        void empty() override;
        void enclose(const glm::vec3 &point) override;

        void transform(const glm::mat4 &transform) override;

        inline const glm::vec3& getMin() const { return min; }
        inline const glm::vec3& getMax() const { return max; }
        inline glm::vec3 getDimensions() const { return getMax() - getMin(); }
        inline glm::vec3 getCenter() const { return (getMin() + getMax()) / 2.0f; }

    private:
        glm::vec3 min;
        glm::vec3 max;
    };
};