#include "VertexArray.hpp"
#include <core/Assert.hpp>
#include <glad/gl.h>

namespace engine {

// ─── GL type helper (backend-only) ───────────────────────────────────────────

static GLenum ToGLType(VertexAttributeType t) noexcept
{
    switch (t) {
        case VertexAttributeType::Float:        return GL_FLOAT;
        case VertexAttributeType::Int:          return GL_INT;
        case VertexAttributeType::UnsignedInt:  return GL_UNSIGNED_INT;
        case VertexAttributeType::Byte:         return GL_BYTE;
        case VertexAttributeType::UnsignedByte: return GL_UNSIGNED_BYTE;
    }
    return GL_FLOAT;
}

// ─── VertexArray ─────────────────────────────────────────────────────────────

VertexArray::VertexArray()
{
    glGenVertexArrays(1, &id_);
}

VertexArray::~VertexArray()
{
    if (id_) glDeleteVertexArrays(1, &id_);
}

VertexArray::VertexArray(VertexArray&& other) noexcept
    : id_(other.id_)
{
    other.id_ = 0;
}

VertexArray& VertexArray::operator=(VertexArray&& other) noexcept
{
    if (this != &other) {
        if (id_) glDeleteVertexArrays(1, &id_);
        id_       = other.id_;
        other.id_ = 0;
    }
    return *this;
}

void VertexArray::AttachVertexBuffer(const Buffer& vbo,
                                     std::span<const VertexAttribute> attrs)
{
    ENGINE_ASSERT(vbo.GetTarget() == BufferTarget::Vertex,
                  "VertexArray: buffer must have Vertex target");

    glBindVertexArray(id_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo.GetID());

    for (const auto& attr : attrs) {
        const GLenum glType = ToGLType(attr.type);

        // Integer attributes use glVertexAttribIPointer (no normalisation step).
        if (attr.type == VertexAttributeType::Int ||
            attr.type == VertexAttributeType::UnsignedInt)
        {
            glVertexAttribIPointer(
                attr.index,
                attr.count,
                glType,
                static_cast<GLsizei>(attr.stride),
                reinterpret_cast<const void*>(attr.offset));
        } else {
            glVertexAttribPointer(
                attr.index,
                attr.count,
                glType,
                attr.normalised ? GL_TRUE : GL_FALSE,
                static_cast<GLsizei>(attr.stride),
                reinterpret_cast<const void*>(attr.offset));
        }

        glEnableVertexAttribArray(attr.index);
        if (attr.divisor > 0)
            glVertexAttribDivisor(attr.index, attr.divisor);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void VertexArray::AttachIndexBuffer(const Buffer& ibo)
{
    ENGINE_ASSERT(ibo.GetTarget() == BufferTarget::Index,
                  "VertexArray: buffer must have Index target");

    // The VAO stores the element buffer binding internally.
    glBindVertexArray(id_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo.GetID());
    glBindVertexArray(0);
}

void VertexArray::Bind()   const { glBindVertexArray(id_); }
void VertexArray::Unbind() const { glBindVertexArray(0);   }

} // namespace engine
