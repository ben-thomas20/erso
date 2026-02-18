#pragma once

#include <renderer/backend/Buffer.hpp>
#include <cstddef>
#include <cstdint>
#include <span>

namespace engine {

// Engine-side vertex attribute type — keeps GL constants out of non-backend code.
enum class VertexAttributeType : std::uint32_t {
    Float,
    Int,
    UnsignedInt,
    Byte,
    UnsignedByte,
};

struct VertexAttribute {
    std::uint32_t       index;
    int                 count;        // components: 1, 2, 3, or 4
    VertexAttributeType type;
    bool                normalised;
    std::uint32_t       stride;       // bytes between consecutive elements
    std::size_t         offset;       // byte offset within each element
    std::uint32_t       divisor = 0;  // 0 = per-vertex, 1 = per-instance
};

class VertexArray {
public:
    VertexArray();
    ~VertexArray();

    VertexArray(const VertexArray&)            = delete;
    VertexArray& operator=(const VertexArray&) = delete;
    VertexArray(VertexArray&&) noexcept;
    VertexArray& operator=(VertexArray&&) noexcept;

    // Bind the VBO and record all attribute pointers into this VAO.
    void AttachVertexBuffer(const Buffer& vbo,
                            std::span<const VertexAttribute> attrs);

    // Record the IBO into this VAO (the binding is stored in the VAO state).
    void AttachIndexBuffer(const Buffer& ibo);

    void Bind()   const;
    void Unbind() const;

    std::uint32_t GetID() const { return id_; }

private:
    std::uint32_t id_ = 0;
};

} // namespace engine
