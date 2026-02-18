#pragma once

#include <resources/GPUMesh.hpp>
#include <renderer/backend/Buffer.hpp>
#include <renderer/backend/VertexArray.hpp>
#include <span>
#include <cstdint>

namespace engine {

// ─── MeshBuffer ───────────────────────────────────────────────────────────────
// A single VBO + IBO + VAO that holds all static mesh geometry for the engine.
// Meshes are appended via Upload() using a simple bump-pointer allocator.
// Once uploaded, geometry is immutable (no remove, no defragment).
//
// All GPUMeshes share this VAO; draw calls use glDrawElementsBaseVertex to
// address each mesh's slice of the shared buffers.
class MeshBuffer {
public:
    // Pre-allocate GPU storage for up to kMaxVertices / kMaxIndices.
    MeshBuffer();

    // Upload a RawMesh into the buffer and return the offsets.
    // Fatal-asserts if capacity is exceeded.
    struct Allocation {
        std::uint32_t baseVertex;
        std::uint32_t baseIndex;
    };
    Allocation Upload(std::span<const MeshVertex>    vertices,
                      std::span<const std::uint32_t> indices);

    // The VAO to bind before any draw call against this buffer.
    std::uint32_t GetVAO() const { return vao_.GetID(); }

    std::uint32_t VertexCount() const { return nextVertex_; }
    std::uint32_t IndexCount()  const { return nextIndex_;  }

    // Capacities (compile-time; bump these if a scene overflows)
    static constexpr std::uint32_t kMaxVertices = 524288u;  // 512 K × 44 B = 22 MB
    static constexpr std::uint32_t kMaxIndices  = 1572864u; //  1.5 M × 4 B  =  6 MB

private:
    Buffer      vbo_;
    Buffer      ibo_;
    VertexArray vao_;

    std::uint32_t nextVertex_ = 0;
    std::uint32_t nextIndex_  = 0;
};

} // namespace engine
