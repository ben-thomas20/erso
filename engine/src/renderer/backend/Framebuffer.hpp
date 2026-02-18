#pragma once

#include <renderer/backend/Texture.hpp>
#include <cstdint>
#include <span>
#include <vector>

namespace engine {

struct AttachmentSpec {
    TextureFormat format;
    TextureFilter filter = TextureFilter::Linear;
    TextureWrap   wrap   = TextureWrap::ClampToEdge;
};

class Framebuffer {
public:
    // colorAttachments must be non-empty.
    // If hasDepthStencil is true a combined depth24/stencil8 attachment is created.
    explicit Framebuffer(std::uint32_t width, std::uint32_t height,
                         std::span<const AttachmentSpec> colorAttachments,
                         bool hasDepthStencil = true);
    ~Framebuffer();

    Framebuffer(const Framebuffer&)            = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;
    Framebuffer(Framebuffer&&) noexcept;
    Framebuffer& operator=(Framebuffer&&) noexcept;

    void Bind() const;
    static void BindDefault();

    // Destroy and recreate all attachments at the new dimensions.
    void Resize(std::uint32_t w, std::uint32_t h);

    const Texture& GetColorAttachment(std::uint32_t index) const;
    const Texture& GetDepthAttachment() const;

    std::uint32_t GetID()     const { return id_; }
    glm::uvec2    GetSize()   const { return size_; }
    std::size_t   ColorCount() const { return colorAttachments_.size(); }

private:
    std::uint32_t id_             = 0;
    glm::uvec2    size_           = {0u, 0u};
    bool          hasDepthStencil_ = true;

    std::vector<AttachmentSpec> colorSpecs_;
    std::vector<Texture>        colorAttachments_;
    Texture                     depthAttachment_;

    void Rebuild();
    void Destroy();
};

} // namespace engine
