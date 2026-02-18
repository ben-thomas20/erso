#pragma once

#include <cstddef>
#include <cstdint>

namespace engine {

enum class BufferTarget { Vertex, Index, Uniform, ShaderStorage };
enum class BufferUsage  { StaticDraw, DynamicDraw, StreamDraw };

class Buffer {
public:
    Buffer(BufferTarget target, BufferUsage usage,
           std::size_t byteSize, const void* data = nullptr);
    ~Buffer();

    Buffer(const Buffer&)            = delete;
    Buffer& operator=(const Buffer&) = delete;
    Buffer(Buffer&&) noexcept;
    Buffer& operator=(Buffer&&) noexcept;

    // Upload a sub-range. Data pointer must remain valid until this returns.
    void Upload(std::size_t offset, std::size_t size, const void* data);

    // Bind to the buffer's own target (GL_ARRAY_BUFFER, etc.).
    void Bind() const;

    // Bind to an indexed binding point — only valid for Uniform and ShaderStorage targets.
    void BindBase(std::uint32_t bindingPoint) const;

    std::uint32_t GetID()     const { return id_; }
    BufferTarget  GetTarget() const { return target_; }
    std::size_t   GetSize()   const { return byteSize_; }

private:
    std::uint32_t id_       = 0;
    BufferTarget  target_;
    std::size_t   byteSize_ = 0;
};

} // namespace engine
