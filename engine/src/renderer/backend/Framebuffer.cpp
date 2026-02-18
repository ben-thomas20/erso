#include "Framebuffer.hpp"
#include <core/Assert.hpp>
#include <core/Log.hpp>
#include <glad/gl.h>

namespace engine {

// ─── Framebuffer ──────────────────────────────────────────────────────────────

Framebuffer::Framebuffer(std::uint32_t width, std::uint32_t height,
                         std::span<const AttachmentSpec> colorAttachments,
                         bool hasDepthStencil)
    : size_           {width, height}
    , hasDepthStencil_(hasDepthStencil)
    , colorSpecs_     (colorAttachments.begin(), colorAttachments.end())
{
    // Depth-only FBOs (empty colorAttachments) are valid for shadow maps.
    ENGINE_ASSERT(!colorSpecs_.empty() || hasDepthStencil,
                  "Framebuffer: must have at least one attachment");
    Rebuild();
}

Framebuffer::~Framebuffer()
{
    Destroy();
}

Framebuffer::Framebuffer(Framebuffer&& other) noexcept
    : id_             (other.id_)
    , size_           (other.size_)
    , hasDepthStencil_(other.hasDepthStencil_)
    , colorSpecs_     (std::move(other.colorSpecs_))
    , colorAttachments_(std::move(other.colorAttachments_))
    , depthAttachment_ (std::move(other.depthAttachment_))
{
    other.id_   = 0;
    other.size_ = {0u, 0u};
}

Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept
{
    if (this != &other) {
        Destroy();
        id_              = other.id_;
        size_            = other.size_;
        hasDepthStencil_ = other.hasDepthStencil_;
        colorSpecs_      = std::move(other.colorSpecs_);
        colorAttachments_= std::move(other.colorAttachments_);
        depthAttachment_ = std::move(other.depthAttachment_);
        other.id_   = 0;
        other.size_ = {0u, 0u};
    }
    return *this;
}

// ─── Bind ─────────────────────────────────────────────────────────────────────

void Framebuffer::Bind() const { glBindFramebuffer(GL_FRAMEBUFFER, id_); }

void Framebuffer::BindDefault() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

// ─── Resize ───────────────────────────────────────────────────────────────────

void Framebuffer::Resize(std::uint32_t w, std::uint32_t h)
{
    if (size_.x == w && size_.y == h) return;
    size_ = {w, h};
    Destroy();
    Rebuild();
}

// ─── Attachment accessors ─────────────────────────────────────────────────────

const Texture& Framebuffer::GetColorAttachment(std::uint32_t index) const
{
    ENGINE_ASSERT(index < colorAttachments_.size(), "Framebuffer: colour attachment index out of range");
    return colorAttachments_[index];
}

const Texture& Framebuffer::GetDepthAttachment() const
{
    ENGINE_ASSERT(hasDepthStencil_, "Framebuffer: no depth attachment");
    return depthAttachment_;
}

// ─── Private ──────────────────────────────────────────────────────────────────

void Framebuffer::Rebuild()
{
    glGenFramebuffers(1, &id_);
    glBindFramebuffer(GL_FRAMEBUFFER, id_);

    colorAttachments_.clear();
    colorAttachments_.reserve(colorSpecs_.size());

    for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(colorSpecs_.size()); ++i) {
        const auto& spec = colorSpecs_[i];
        colorAttachments_.push_back(
            Texture::Create(size_.x, size_.y,
                            spec.format, spec.filter, spec.filter, spec.wrap));
        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               GL_COLOR_ATTACHMENT0 + i,
                               GL_TEXTURE_2D,
                               colorAttachments_.back().GetID(), 0);
    }

    if (hasDepthStencil_) {
        const bool depthOnly = colorSpecs_.empty();
        const TextureFormat depthFmt = depthOnly
            ? TextureFormat::Depth32F          // shadow map: pure depth, samplers work cleanly
            : TextureFormat::Depth24Stencil8;  // G-buffer / general FBO
        const GLenum attachment = depthOnly
            ? GL_DEPTH_ATTACHMENT
            : GL_DEPTH_STENCIL_ATTACHMENT;

        depthAttachment_ = Texture::Create(
            size_.x, size_.y,
            depthFmt,
            TextureFilter::Nearest, TextureFilter::Nearest,
            TextureWrap::ClampToEdge);
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment,
                               GL_TEXTURE_2D,
                               depthAttachment_.GetID(), 0);
    }

    // Configure draw buffers: depth-only FBOs use GL_NONE.
    if (colorSpecs_.empty()) {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    } else {
        std::vector<GLenum> drawBuffers;
        drawBuffers.reserve(colorSpecs_.size());
        for (std::size_t i = 0; i < colorSpecs_.size(); ++i)
            drawBuffers.push_back(static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + i));
        glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
    }

    const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        LOG_ERROR("Framebuffer incomplete: status={:#x}", static_cast<unsigned>(status));
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::Destroy()
{
    colorAttachments_.clear();
    depthAttachment_ = Texture{};
    if (id_) {
        glDeleteFramebuffers(1, &id_);
        id_ = 0;
    }
}

} // namespace engine
