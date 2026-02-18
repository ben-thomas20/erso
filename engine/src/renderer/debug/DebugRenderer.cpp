#include <renderer/debug/DebugRenderer.hpp>
#include <core/Assert.hpp>
#include <core/Log.hpp>

#include <glad/gl.h>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>
#include <cmath>

#ifndef ENGINE_ASSET_DIR
#  define ENGINE_ASSET_DIR "assets"
#endif
#define ASSET(rel) ENGINE_ASSET_DIR "/" rel

namespace engine {

DebugRenderer::DebugRenderer()
    : vbo_(BufferTarget::Vertex, BufferUsage::StreamDraw,
           kMaxVertices * sizeof(DebugVertex))
    , shader_(Shader::FromFiles(ASSET("shaders/debug/debug.vert"),
                                ASSET("shaders/debug/debug.frag")))
{
    vertices_.reserve(512);

    const std::array<VertexAttribute, 2> attrs = {{
        {0, 3, VertexAttributeType::Float, false,
         static_cast<std::uint32_t>(sizeof(DebugVertex)),
         offsetof(DebugVertex, pos)},
        {1, 4, VertexAttributeType::Float, false,
         static_cast<std::uint32_t>(sizeof(DebugVertex)),
         offsetof(DebugVertex, color)},
    }};

    vao_.AttachVertexBuffer(vbo_, attrs);

    ENGINE_ASSERT(shader_.IsValid(), "Debug shader failed to compile");
    LOG_INFO("DebugRenderer ready");
}

void DebugRenderer::DrawLine(glm::vec3 a, glm::vec3 b, glm::vec4 color)
{
    if (vertices_.size() + 2 > kMaxVertices) return;
    vertices_.push_back({a, color});
    vertices_.push_back({b, color});
}

void DebugRenderer::DrawAABB(const AABB& aabb, const glm::mat4& transform, glm::vec4 color)
{
    const glm::vec3& mn = aabb.min;
    const glm::vec3& mx = aabb.max;

    // 8 corners of the AABB in local space, transformed to world space.
    std::array<glm::vec3, 8> c;
    for (int i = 0; i < 8; ++i) {
        c[i] = glm::vec3(transform * glm::vec4(
            (i & 1) ? mx.x : mn.x,
            (i & 2) ? mx.y : mn.y,
            (i & 4) ? mx.z : mn.z,
            1.0f));
    }

    // 12 edges: connect corners that differ by exactly one bit.
    // Bottom face: 0-1, 0-2, 3-1, 3-2
    // Top face:    4-5, 4-6, 7-5, 7-6
    // Verticals:   0-4, 1-5, 2-6, 3-7
    const int edges[12][2] = {
        {0,1},{0,2},{3,1},{3,2},
        {4,5},{4,6},{7,5},{7,6},
        {0,4},{1,5},{2,6},{3,7}
    };
    for (const auto& e : edges)
        DrawLine(c[e[0]], c[e[1]], color);
}

void DebugRenderer::DrawSphere(glm::vec3 center, float radius, glm::vec4 color, int segments)
{
    // Draw three great circles (XY, XZ, YZ planes).
    const float step = glm::two_pi<float>() / static_cast<float>(segments);
    for (int axis = 0; axis < 3; ++axis) {
        for (int i = 0; i < segments; ++i) {
            const float a0 = static_cast<float>(i)     * step;
            const float a1 = static_cast<float>(i + 1) * step;
            glm::vec3 p0{}, p1{};
            if (axis == 0) { // XY
                p0 = {glm::cos(a0), glm::sin(a0), 0.f};
                p1 = {glm::cos(a1), glm::sin(a1), 0.f};
            } else if (axis == 1) { // XZ
                p0 = {glm::cos(a0), 0.f, glm::sin(a0)};
                p1 = {glm::cos(a1), 0.f, glm::sin(a1)};
            } else { // YZ
                p0 = {0.f, glm::cos(a0), glm::sin(a0)};
                p1 = {0.f, glm::cos(a1), glm::sin(a1)};
            }
            DrawLine(center + p0 * radius, center + p1 * radius, color);
        }
    }
}

void DebugRenderer::DrawFrustum(const Frustum& frustum, glm::vec4 color)
{
    // Reconstruct the 8 frustum corners from the 6 planes by intersecting
    // triples. A simpler approach: the user typically provides the frustum
    // built from a VP matrix, so we can extract the corners from the
    // inverse of that matrix. Since we only store planes, use a helper that
    // finds plane intersections for the 8 corners:
    // near-bottom-left, near-bottom-right, near-top-left,  near-top-right,
    // far-bottom-left,  far-bottom-right,  far-top-left,   far-top-right.
    //
    // We use a known-NDC-corners approach: transform the 8 NDC corners
    // through the inverse VP.  But we don't store the VP inverse here.
    // Instead, intersect each corner as the meet of three planes.

    // Plane indices: 0=left,1=right,2=bottom,3=top,4=near,5=far
    // 8 corners are intersections of: (near|far) x (left|right) x (bottom|top)
    const int cornerPlanes[8][3] = {
        {4, 0, 2}, // near, left,  bottom
        {4, 1, 2}, // near, right, bottom
        {4, 0, 3}, // near, left,  top
        {4, 1, 3}, // near, right, top
        {5, 0, 2}, // far,  left,  bottom
        {5, 1, 2}, // far,  right, bottom
        {5, 0, 3}, // far,  left,  top
        {5, 1, 3}, // far,  right, top
    };

    std::array<glm::vec3, 8> corners;
    for (int i = 0; i < 8; ++i) {
        const glm::vec4& p0 = frustum.planes[cornerPlanes[i][0]];
        const glm::vec4& p1 = frustum.planes[cornerPlanes[i][1]];
        const glm::vec4& p2 = frustum.planes[cornerPlanes[i][2]];

        const glm::vec3 n0(p0), n1(p1), n2(p2);
        const glm::vec3 cross01 = glm::cross(n0, n1);
        const glm::vec3 cross12 = glm::cross(n1, n2);
        const glm::vec3 cross20 = glm::cross(n2, n0);

        const float denom = glm::dot(n0, cross12);
        if (std::abs(denom) < 1e-8f) {
            corners[i] = glm::vec3(0.f);
            continue;
        }
        corners[i] = -(p0.w * cross12 + p1.w * cross20 + p2.w * cross01) / denom;
    }

    // Draw the 12 edges of the frustum (same connectivity as AABB corners).
    const int edges[12][2] = {
        {0,1},{0,2},{3,1},{3,2},  // near face
        {4,5},{4,6},{7,5},{7,6},  // far face
        {0,4},{1,5},{2,6},{3,7}   // connecting edges
    };
    for (const auto& e : edges)
        DrawLine(corners[e[0]], corners[e[1]], color);
}

void DebugRenderer::FlushAndClear(const glm::mat4& viewProjection)
{
    if (vertices_.empty()) return;

    // Upload only the used portion of the VBO.
    const std::size_t byteCount = vertices_.size() * sizeof(DebugVertex);
    vbo_.Upload(0, byteCount, vertices_.data());

    shader_.Bind();
    shader_.SetMat4("u_ViewProjection", viewProjection);

    vao_.Bind();
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(vertices_.size()));
    vao_.Unbind();

    vertices_.clear();
}

} // namespace engine
