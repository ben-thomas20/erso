#include <core/Frustum.hpp>
#include <glm/geometric.hpp>
#include <array>

namespace engine {

Frustum Frustum::FromViewProjection(const glm::mat4& vp)
{
    // GLM matrices are column-major: vp[c][r] = column c, row r.
    // Extract the four rows as vec4s for Gribb/Hartmann plane extraction.
    auto row = [&](int r) -> glm::vec4 {
        return glm::vec4(vp[0][r], vp[1][r], vp[2][r], vp[3][r]);
    };

    const glm::vec4 r0 = row(0), r1 = row(1), r2 = row(2), r3 = row(3);

    Frustum f;
    f.planes[0] = r3 + r0;  // left
    f.planes[1] = r3 - r0;  // right
    f.planes[2] = r3 + r1;  // bottom
    f.planes[3] = r3 - r1;  // top
    f.planes[4] = r3 + r2;  // near
    f.planes[5] = r3 - r2;  // far

    // Normalise so ContainsSphere distance tests are metrically correct.
    for (auto& p : f.planes) {
        const float len = glm::length(glm::vec3(p));
        if (len > 1e-6f) p /= len;
    }
    return f;
}

bool Frustum::ContainsSphere(glm::vec3 center, float radius) const
{
    for (const auto& p : planes) {
        if (p.x * center.x + p.y * center.y + p.z * center.z + p.w < -radius)
            return false;
    }
    return true;
}

bool Frustum::ContainsAABB(const AABB& aabb, const glm::mat4& model) const
{
    // Transform all 8 AABB corners to world space.
    const glm::vec3& mn = aabb.min;
    const glm::vec3& mx = aabb.max;

    std::array<glm::vec3, 8> corners;
    for (int i = 0; i < 8; ++i) {
        corners[i] = glm::vec3(model * glm::vec4(
            (i & 1) ? mx.x : mn.x,
            (i & 2) ? mx.y : mn.y,
            (i & 4) ? mx.z : mn.z,
            1.0f));
    }

    // For each plane, if ALL 8 corners are in the negative half-space the
    // AABB is entirely outside — early reject.
    for (const auto& p : planes) {
        bool allOutside = true;
        for (const auto& c : corners) {
            if (p.x * c.x + p.y * c.y + p.z * c.z + p.w >= 0.0f) {
                allOutside = false;
                break;
            }
        }
        if (allOutside) return false;
    }
    return true;
}

} // namespace engine
