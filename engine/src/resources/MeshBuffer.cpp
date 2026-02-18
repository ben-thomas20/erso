#include <resources/MeshBuffer.hpp>
#include <core/Assert.hpp>
#include <core/Log.hpp>

namespace engine {

MeshBuffer::MeshBuffer()
    : vbo_(BufferTarget::Vertex, BufferUsage::DynamicDraw,
           static_cast<std::size_t>(kMaxVertices) * sizeof(MeshVertex))
    , ibo_(BufferTarget::Index,  BufferUsage::DynamicDraw,
           static_cast<std::size_t>(kMaxIndices)  * sizeof(std::uint32_t))
{
    // Attach the VBO and IBO to the shared VAO once; all meshes reuse this.
    vao_.AttachVertexBuffer(vbo_, std::span(kMeshVertexAttributes));
    vao_.AttachIndexBuffer(ibo_);

    LOG_INFO("MeshBuffer: allocated {:.1f} MB VBO + {:.1f} MB IBO",
             static_cast<float>(kMaxVertices * sizeof(MeshVertex))  / (1024.f * 1024.f),
             static_cast<float>(kMaxIndices  * sizeof(std::uint32_t))/ (1024.f * 1024.f));
}

MeshBuffer::Allocation MeshBuffer::Upload(std::span<const MeshVertex>    vertices,
                                          std::span<const std::uint32_t> indices)
{
    ENGINE_ASSERT(nextVertex_ + vertices.size() <= kMaxVertices,
                  "MeshBuffer: vertex capacity exceeded");
    ENGINE_ASSERT(nextIndex_  + indices.size()  <= kMaxIndices,
                  "MeshBuffer: index capacity exceeded");

    const std::size_t vertOffset = static_cast<std::size_t>(nextVertex_) * sizeof(MeshVertex);
    const std::size_t idxOffset  = static_cast<std::size_t>(nextIndex_)  * sizeof(std::uint32_t);

    vbo_.Upload(vertOffset, vertices.size_bytes(), vertices.data());
    ibo_.Upload(idxOffset,  indices.size_bytes(),  indices.data());

    const Allocation alloc{ nextVertex_, nextIndex_ };
    nextVertex_ += static_cast<std::uint32_t>(vertices.size());
    nextIndex_  += static_cast<std::uint32_t>(indices.size());
    return alloc;
}

} // namespace engine
