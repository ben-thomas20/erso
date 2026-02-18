#pragma once

#include <glm/vec3.hpp>
#include <glm/common.hpp>
#include <limits>

namespace engine {

// Axis-Aligned Bounding Box in local (model) space.
struct AABB {
    glm::vec3 min = glm::vec3( std::numeric_limits<float>::max());
    glm::vec3 max = glm::vec3(-std::numeric_limits<float>::max());

    bool IsValid() const { return min.x <= max.x; }

    glm::vec3 Center()  const { return (min + max) * 0.5f; }
    glm::vec3 Extents() const { return (max - min) * 0.5f; }
    glm::vec3 Size()    const { return max - min; }

    void Expand(const glm::vec3& point)
    {
        min = glm::min(min, point);
        max = glm::max(max, point);
    }

    void Expand(const AABB& other)
    {
        Expand(other.min);
        Expand(other.max);
    }
};

} // namespace engine
