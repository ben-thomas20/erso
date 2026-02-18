#include "Buffer.hpp"
#include <core/Assert.hpp>
#include <glad/gl.h>

// GL_SHADER_STORAGE_BUFFER is core in 4.3; macOS tops out at 4.1.
// Define the constant so the enum value compiles everywhere.
// Actual SSBO usage will be gated behind a GL 4.3 capability check at runtime.
#ifndef GL_SHADER_STORAGE_BUFFER
#  define GL_SHADER_STORAGE_BUFFER 0x90D2u
#endif

namespace engine {

// ─── GL enum helpers (backend-only) ──────────────────────────────────────────

static GLenum ToGLTarget(BufferTarget t) noexcept
{
    switch (t) {
        case BufferTarget::Vertex:       return GL_ARRAY_BUFFER;
        case BufferTarget::Index:        return GL_ELEMENT_ARRAY_BUFFER;
        case BufferTarget::Uniform:      return GL_UNIFORM_BUFFER;
        case BufferTarget::ShaderStorage: return GL_SHADER_STORAGE_BUFFER;
    }
    return GL_ARRAY_BUFFER;
}

static GLenum ToGLUsage(BufferUsage u) noexcept
{
    switch (u) {
        case BufferUsage::StaticDraw:  return GL_STATIC_DRAW;
        case BufferUsage::DynamicDraw: return GL_DYNAMIC_DRAW;
        case BufferUsage::StreamDraw:  return GL_STREAM_DRAW;
    }
    return GL_STATIC_DRAW;
}

// ─── Buffer ──────────────────────────────────────────────────────────────────

Buffer::Buffer(BufferTarget target, BufferUsage usage,
               std::size_t byteSize, const void* data)
    : target_(target)
    , byteSize_(byteSize)
{
    glGenBuffers(1, &id_);
    glBindBuffer(ToGLTarget(target_), id_);
    glBufferData(ToGLTarget(target_), static_cast<GLsizeiptr>(byteSize),
                 data, ToGLUsage(usage));
    glBindBuffer(ToGLTarget(target_), 0);
}

Buffer::~Buffer()
{
    if (id_) glDeleteBuffers(1, &id_);
}

Buffer::Buffer(Buffer&& other) noexcept
    : id_(other.id_), target_(other.target_), byteSize_(other.byteSize_)
{
    other.id_       = 0;
    other.byteSize_ = 0;
}

Buffer& Buffer::operator=(Buffer&& other) noexcept
{
    if (this != &other) {
        if (id_) glDeleteBuffers(1, &id_);
        id_       = other.id_;
        target_   = other.target_;
        byteSize_ = other.byteSize_;
        other.id_       = 0;
        other.byteSize_ = 0;
    }
    return *this;
}

void Buffer::Upload(std::size_t offset, std::size_t size, const void* data)
{
    ENGINE_ASSERT(offset + size <= byteSize_, "Buffer::Upload out of range");
    glBindBuffer(ToGLTarget(target_), id_);
    glBufferSubData(ToGLTarget(target_),
                    static_cast<GLintptr>(offset),
                    static_cast<GLsizeiptr>(size),
                    data);
    glBindBuffer(ToGLTarget(target_), 0);
}

void Buffer::Bind() const
{
    glBindBuffer(ToGLTarget(target_), id_);
}

void Buffer::BindBase(std::uint32_t bindingPoint) const
{
    ENGINE_ASSERT(target_ == BufferTarget::Uniform ||
                  target_ == BufferTarget::ShaderStorage,
                  "BindBase is only valid for Uniform and ShaderStorage buffers");
    glBindBufferBase(ToGLTarget(target_), bindingPoint, id_);
}

} // namespace engine
