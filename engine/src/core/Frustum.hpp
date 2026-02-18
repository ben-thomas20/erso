#pragma once

#include <core/Geometry.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <array>

namespace engine {

// ─── Frustum ──────────────────────────────────────────────────────────────────
// Represents the six planes of a view frustum in world space.
// Each plane is stored as (normal.xyz, d) where the positive half-space is
// considered "inside": dot(normal, point) + d >= 0 means the point is inside.
struct Frustum {
    std::array<glm::vec4, 6> planes; // left, right, bottom, top, near, far

    // Extract the six planes from a combined view-projection matrix using the
    // Gribb/Hartmann method.  Planes are normalised so distance tests are
    // accurate (required by ContainsSphere).
    static Frustum FromViewProjection(const glm::mat4& vp);

    // Returns true if the sphere may overlap the frustum (conservative — no
    // false negatives, rare false positives near corners).
    bool ContainsSphere(glm::vec3 center, float radius) const;

    // Returns true if the AABB (in local space, transformed by model) may
    // overlap the frustum.  Returns false only when the AABB is definitively
    // outside at least one plane.
    bool ContainsAABB(const AABB& aabb, const glm::mat4& model) const;
};

} // namespace engine
