#include "Texture.hpp"
#include <core/Assert.hpp>
#include <core/Log.hpp>
#include <glad/gl.h>
#include <stb_image.h>

namespace engine {

// ─── GL enum helpers (backend-only) ──────────────────────────────────────────

struct GLFormats {
    GLenum internalFormat;
    GLenum baseFormat;
    GLenum dataType;
};

static GLFormats ToGLFormats(TextureFormat fmt) noexcept
{
    switch (fmt) {
        case TextureFormat::R8:             return {GL_R8,                GL_RED,          GL_UNSIGNED_BYTE};
        case TextureFormat::RG8:            return {GL_RG8,               GL_RG,           GL_UNSIGNED_BYTE};
        case TextureFormat::RGB8:           return {GL_RGB8,              GL_RGB,          GL_UNSIGNED_BYTE};
        case TextureFormat::RGBA8:          return {GL_RGBA8,             GL_RGBA,         GL_UNSIGNED_BYTE};
        case TextureFormat::RGB16F:         return {GL_RGB16F,            GL_RGB,          GL_HALF_FLOAT};
        case TextureFormat::RGBA16F:        return {GL_RGBA16F,           GL_RGBA,         GL_HALF_FLOAT};
        case TextureFormat::Depth24Stencil8:return {GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL,GL_UNSIGNED_INT_24_8};
        case TextureFormat::Depth32F:       return {GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT};
    }
    return {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE};
}

static GLenum ToGLFilter(TextureFilter f) noexcept
{
    switch (f) {
        case TextureFilter::Nearest:            return GL_NEAREST;
        case TextureFilter::Linear:             return GL_LINEAR;
        case TextureFilter::LinearMipmapLinear: return GL_LINEAR_MIPMAP_LINEAR;
    }
    return GL_LINEAR;
}

static GLenum ToGLWrap(TextureWrap w) noexcept
{
    switch (w) {
        case TextureWrap::Repeat:        return GL_REPEAT;
        case TextureWrap::ClampToEdge:   return GL_CLAMP_TO_EDGE;
        case TextureWrap::ClampToBorder: return GL_CLAMP_TO_BORDER;
    }
    return GL_REPEAT;
}

// ─── Factory — FromFile ───────────────────────────────────────────────────────

Texture Texture::FromFile(const std::filesystem::path& path, bool genMipmaps)
{
    stbi_set_flip_vertically_on_load(true);

    int w = 0, h = 0, channels = 0;
    unsigned char* pixels = stbi_load(path.string().c_str(), &w, &h, &channels, 0);

    if (!pixels) {
        LOG_ERROR("Texture::FromFile — stbi_load failed: {} ({})",
                  stbi_failure_reason(), path.string());
        return {};
    }

    TextureFormat fmt = TextureFormat::RGBA8;
    switch (channels) {
        case 1: fmt = TextureFormat::R8;    break;
        case 2: fmt = TextureFormat::RG8;   break;
        case 3: fmt = TextureFormat::RGB8;  break;
        case 4: fmt = TextureFormat::RGBA8; break;
        default: break;
    }

    Texture tex = FromData(static_cast<std::uint32_t>(w),
                           static_cast<std::uint32_t>(h),
                           fmt, pixels, genMipmaps);
    stbi_image_free(pixels);
    return tex;
}

// ─── Factory — Create (empty, FBO-ready) ──────────────────────────────────────

Texture Texture::Create(std::uint32_t w, std::uint32_t h,
                        TextureFormat format,
                        TextureFilter minFilter,
                        TextureFilter magFilter,
                        TextureWrap   wrap)
{
    const auto [intFmt, baseFmt, dataType] = ToGLFormats(format);

    std::uint32_t id = 0;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexImage2D(GL_TEXTURE_2D, 0,
                 static_cast<GLint>(intFmt),
                 static_cast<GLsizei>(w), static_cast<GLsizei>(h),
                 0, baseFmt, dataType, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    static_cast<GLint>(ToGLFilter(minFilter)));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                    static_cast<GLint>(ToGLFilter(magFilter)));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    static_cast<GLint>(ToGLWrap(wrap)));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                    static_cast<GLint>(ToGLWrap(wrap)));

    // For depth formats: disable hardware comparison so sampler2D returns the
    // raw depth value in the red channel instead of a 0/1 comparison result.
    if (format == TextureFormat::Depth24Stencil8 || format == TextureFormat::Depth32F) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    return Texture(id, {w, h});
}

// ─── Factory — FromData ───────────────────────────────────────────────────────

Texture Texture::FromData(std::uint32_t w, std::uint32_t h,
                          TextureFormat format,
                          const void*   pixels,
                          bool          genMipmaps)
{
    const auto [intFmt, baseFmt, dataType] = ToGLFormats(format);

    std::uint32_t id = 0;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexImage2D(GL_TEXTURE_2D, 0,
                 static_cast<GLint>(intFmt),
                 static_cast<GLsizei>(w), static_cast<GLsizei>(h),
                 0, baseFmt, dataType, pixels);

    const GLenum minFilter = genMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(minFilter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    if (genMipmaps) glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
    return Texture(id, {w, h});
}

// ─── Lifecycle ────────────────────────────────────────────────────────────────

Texture::~Texture()
{
    if (id_) glDeleteTextures(1, &id_);
}

Texture::Texture(Texture&& other) noexcept
    : id_(other.id_), size_(other.size_)
{
    other.id_   = 0;
    other.size_ = {0u, 0u};
}

Texture& Texture::operator=(Texture&& other) noexcept
{
    if (this != &other) {
        if (id_) glDeleteTextures(1, &id_);
        id_       = other.id_;
        size_     = other.size_;
        other.id_   = 0;
        other.size_ = {0u, 0u};
    }
    return *this;
}

void Texture::Bind(std::uint32_t unit) const
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, id_);
}

} // namespace engine
