#pragma once

#include <core/Frustum.hpp>
#include <core/Geometry.hpp>
#include <renderer/backend/Buffer.hpp>
#include <renderer/backend/VertexArray.hpp>
#include <renderer/backend/Shader.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <vector>
#include <cstddef>

namespace engine {

// ─── DebugRenderer ────────────────────────────────────────────────────────────
// Immediate-mode debug geometry renderer.
//
// All geometry submitted in a frame is batched into one dynamic VBO and
// flushed in a single glDrawArrays(GL_LINES, ...) call.
//
// Typical use:
//   debugRenderer.DrawAABB(mesh.localBounds, worldMatrix, {0,1,0,1});
//   debugRenderer.DrawFrustum(frustum, {1,1,0,1});
//   debugRenderer.FlushAndClear(viewProjectionMatrix);
class DebugRenderer {
public:
    DebugRenderer();

    void DrawLine  (glm::vec3 a, glm::vec3 b, glm::vec4 color);
    void DrawAABB  (const AABB& aabb, const glm::mat4& transform, glm::vec4 color);
    void DrawSphere(glm::vec3 center, float radius, glm::vec4 color, int segments = 16);
    void DrawFrustum(const Frustum& frustum, glm::vec4 color);

    // Upload batched geometry to GPU, issue one GL_LINES draw call, then clear.
    void FlushAndClear(const glm::mat4& viewProjection);

    bool IsEmpty() const { return vertices_.empty(); }

    // Public so the .cpp can use offsetof without friendship.
    struct DebugVertex {
        glm::vec3 pos;
        glm::vec4 color;
    };

private:

    static constexpr std::size_t kMaxVertices = 65536;

    std::vector<DebugVertex> vertices_;
    Buffer                   vbo_;
    VertexArray              vao_;
    Shader                   shader_;
};

} // namespace engine
