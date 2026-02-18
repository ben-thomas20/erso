#pragma once

#include <filesystem>
#include <string_view>
#include <vector>
#include <cstdint>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace engine {

class Shader {
public:
    // Build a shader program from source files.
    // The ShaderPreprocessor resolves any #include directives.
    // geom may be empty (no geometry stage).
    static Shader FromFiles(const std::filesystem::path& vert,
                            const std::filesystem::path& frag,
                            const std::filesystem::path& geom = {});

    ~Shader();

    Shader(const Shader&)            = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&&) noexcept;
    Shader& operator=(Shader&&) noexcept;

    void Bind() const;

    // Convenience uniform setters — prefer UBOs for per-frame / per-object data;
    // these are only for one-off uniforms that don't justify a full UBO.
    void SetInt    (std::string_view name, int v)              const;
    void SetFloat  (std::string_view name, float v)            const;
    void SetVec3   (std::string_view name, glm::vec3 v)        const;
    void SetMat4   (std::string_view name, const glm::mat4& m) const;
    void SetTexture(std::string_view name, int unit)           const;

    // Recompile from the original source files.
    // Returns true on success.  On failure, the previous program is preserved
    // and false is returned — the engine never crashes on a shader typo.
    bool Reload();

    bool          IsValid() const { return id_ != 0; }
    std::uint32_t GetID()   const { return id_; }

    // Path accessors for hot-reload tracking.
    const std::filesystem::path& VertPath() const { return vertPath_; }
    const std::filesystem::path& FragPath() const { return fragPath_; }
    const std::filesystem::path& GeomPath() const { return geomPath_; }

    // All .glsl files read during the last successful compile (main + includes).
    // Updated by Reload(); used by ResourceManager::PollShaderReload.
    const std::vector<std::filesystem::path>& GetDependencies() const { return deps_; }

private:
    Shader() = default;

    std::uint32_t id_ = 0;
    std::filesystem::path vertPath_;
    std::filesystem::path fragPath_;
    std::filesystem::path geomPath_;
    std::vector<std::filesystem::path> deps_;

    // Compile one shader stage from preprocessed source.
    // Returns 0 on failure (error is logged).
    std::uint32_t CompileStage(std::uint32_t glType, const std::string& src) const;

    // Link compiled stages into a program.
    // Deletes the stage objects regardless of success.
    // Returns 0 on failure.
    static std::uint32_t LinkProgram(std::uint32_t vert, std::uint32_t frag,
                                     std::uint32_t geom = 0);

    // Fetch a uniform location (cached implicitly by the GL driver).
    int UniformLocation(std::string_view name) const;
};

} // namespace engine
