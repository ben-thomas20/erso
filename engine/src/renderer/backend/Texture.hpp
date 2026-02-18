#pragma once

#include <cstdint>
#include <filesystem>

#include <glm/vec2.hpp>

namespace engine {

enum class TextureFormat {
    R8, RG8, RGB8, RGBA8,
    RGB16F, RGBA16F,
    Depth24Stencil8,   // FBO depth+stencil attachment
    Depth32F,          // shadow map (pure depth, samplers get red channel directly)
};
enum class TextureFilter { Nearest, Linear, LinearMipmapLinear };
enum class TextureWrap   { Repeat, ClampToEdge, ClampToBorder };

class Texture {
public:
    // Default-constructs an invalid texture (id_ == 0).
    // Used by Framebuffer to initialise member textures before Rebuild().
    Texture() = default;

    // Load from a file using stb_image.  sRGB conversion is applied when the
    // source has 3 or 4 channels and genMipmaps is true.
    static Texture FromFile(const std::filesystem::path& path,
                            bool genMipmaps = true);

    // Create an empty texture (suitable for FBO attachments).
    static Texture Create(std::uint32_t w, std::uint32_t h,
                          TextureFormat format,
                          TextureFilter minFilter,
                          TextureFilter magFilter,
                          TextureWrap   wrap);

    // Upload raw pixel data directly.  format must describe the pixel layout
    // of the provided buffer (e.g. RGBA8 → 4 bytes per pixel).
    static Texture FromData(std::uint32_t w, std::uint32_t h,
                            TextureFormat format,
                            const void*   pixels,
                            bool          genMipmaps = true);

    ~Texture();

    Texture(const Texture&)            = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&&) noexcept;
    Texture& operator=(Texture&&) noexcept;

    void Bind(std::uint32_t unit) const;

    std::uint32_t GetID()   const { return id_; }
    glm::uvec2    GetSize() const { return size_; }
    bool          IsValid() const { return id_ != 0; }

private:
    std::uint32_t id_   = 0;
    glm::uvec2    size_ = {0u, 0u};

    // Internal constructor used by the factory methods.
    Texture(std::uint32_t id, glm::uvec2 size) : id_(id), size_(size) {}
};

} // namespace engine
