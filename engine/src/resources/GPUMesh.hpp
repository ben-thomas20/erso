#pragma once

#include <core/Geometry.hpp>
#include <renderer/backend/Buffer.hpp>
#include <renderer/backend/VertexArray.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <cstdint>
#include <vector>

namespace engine {

// Standard vertex layout used by all meshes loaded through MeshLoader.
// Attribute locations must match the geometry shaders:
//   location 0 → position
//   location 1 → normal
//   location 2 → uv
//   location 3 → tangent
struct MeshVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::vec3 tangent;
};

// Vertex attributes that describe MeshVertex to a VertexArray.
inline constexpr std::array<VertexAttribute, 4> kMeshVertexAttributes = {{
    {0, 3, VertexAttributeType::Float, false,
     static_cast<std::uint32_t>(sizeof(MeshVertex)),
     offsetof(MeshVertex, position)},
    {1, 3, VertexAttributeType::Float, false,
     static_cast<std::uint32_t>(sizeof(MeshVertex)),
     offsetof(MeshVertex, normal)},
    {2, 2, VertexAttributeType::Float, false,
     static_cast<std::uint32_t>(sizeof(MeshVertex)),
     offsetof(MeshVertex, uv)},
    {3, 3, VertexAttributeType::Float, false,
     static_cast<std::uint32_t>(sizeof(MeshVertex)),
     offsetof(MeshVertex, tangent)},
}};

// ─── RawMesh (CPU-side) ───────────────────────────────────────────────────────
// Intermediate representation before upload to the GPU mega-buffer.
// Produced by MeshLoader; consumed by ResourceManager::AddMesh.
struct RawMesh {
    std::vector<MeshVertex>    vertices;
    std::vector<std::uint32_t> indices;
    AABB                       localBounds;
};

// ─── GPUMesh (GPU-side, lightweight) ─────────────────────────────────────────
// After upload, a mesh is identified by its offsets into the shared MeshBuffer
// (owned by ResourceManager).  All meshes share one VAO; drawing uses
// glDrawElementsBaseVertex so indices are re-based per mesh.
struct GPUMesh {
    std::uint32_t sharedVAOID = 0;   // the ResourceManager's mega-buffer VAO
    std::uint32_t baseVertex  = 0;   // first vertex in the shared VBO
    std::uint32_t baseIndex   = 0;   // first index in the shared IBO
    std::uint32_t indexCount  = 0;
    AABB          localBounds;

    GPUMesh() = default;
    GPUMesh(GPUMesh&&) noexcept            = default;
    GPUMesh& operator=(GPUMesh&&) noexcept = default;
    GPUMesh(const GPUMesh&)                = delete;
    GPUMesh& operator=(const GPUMesh&)     = delete;
};

} // namespace engine
