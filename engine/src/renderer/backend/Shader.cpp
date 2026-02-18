#include "Shader.hpp"
#include <resources/ShaderPreprocessor.hpp>
#include <core/Log.hpp>

#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>

#include <stdexcept>

namespace engine {

// ─── Factory ──────────────────────────────────────────────────────────────────

Shader Shader::FromFiles(const std::filesystem::path& vert,
                         const std::filesystem::path& frag,
                         const std::filesystem::path& geom)
{
    Shader s;
    s.vertPath_ = vert;
    s.fragPath_ = frag;
    s.geomPath_ = geom;

    if (!s.Reload()) {
        LOG_ERROR("Shader::FromFiles — initial compilation failed for '{}' / '{}'",
                  vert.string(), frag.string());
    }
    return s;
}

// ─── Lifecycle ────────────────────────────────────────────────────────────────

Shader::~Shader()
{
    if (id_) glDeleteProgram(id_);
}

Shader::Shader(Shader&& other) noexcept
    : id_      (other.id_)
    , vertPath_(std::move(other.vertPath_))
    , fragPath_(std::move(other.fragPath_))
    , geomPath_(std::move(other.geomPath_))
    , deps_    (std::move(other.deps_))
{
    other.id_ = 0;
}

Shader& Shader::operator=(Shader&& other) noexcept
{
    if (this != &other) {
        if (id_) glDeleteProgram(id_);
        id_       = other.id_;
        vertPath_ = std::move(other.vertPath_);
        fragPath_ = std::move(other.fragPath_);
        geomPath_ = std::move(other.geomPath_);
        deps_     = std::move(other.deps_);
        other.id_ = 0;
    }
    return *this;
}

// ─── Reload ───────────────────────────────────────────────────────────────────

bool Shader::Reload()
{
    std::vector<std::filesystem::path> newDeps;

    auto processWith = [&](const std::filesystem::path& path) -> std::string {
        auto result = ShaderPreprocessor::Process(path);
        for (auto& d : result.dependencies) newDeps.push_back(d);
        return std::move(result.source);
    };

    std::string vertSrc, fragSrc;
    try {
        vertSrc = processWith(vertPath_);
        fragSrc = processWith(fragPath_);
    } catch (const std::exception& e) {
        LOG_ERROR("Shader::Reload — preprocessor error: {}", e.what());
        return false;
    }

    const std::uint32_t vertObj = CompileStage(GL_VERTEX_SHADER,   vertSrc);
    const std::uint32_t fragObj = CompileStage(GL_FRAGMENT_SHADER, fragSrc);
    if (!vertObj || !fragObj) {
        if (vertObj) glDeleteShader(vertObj);
        if (fragObj) glDeleteShader(fragObj);
        return false;
    }

    std::uint32_t geomObj = 0;
    if (!geomPath_.empty()) {
        try {
            const std::string geomSrc = processWith(geomPath_);
            geomObj = CompileStage(GL_GEOMETRY_SHADER, geomSrc);
        } catch (const std::exception& e) {
            LOG_ERROR("Shader::Reload — geometry preprocessor error: {}", e.what());
        }
        if (!geomObj) {
            glDeleteShader(vertObj);
            glDeleteShader(fragObj);
            return false;
        }
    }

    const std::uint32_t newProg = LinkProgram(vertObj, fragObj, geomObj);
    glDeleteShader(vertObj);
    glDeleteShader(fragObj);
    if (geomObj) glDeleteShader(geomObj);

    if (!newProg) return false;

    if (id_) glDeleteProgram(id_);
    id_   = newProg;
    deps_ = std::move(newDeps);  // update dependency list on success

    LOG_TRACE("Shader reloaded (prog={}): {} / {}", id_,
              vertPath_.filename().string(), fragPath_.filename().string());
    return true;
}

// ─── Bind / uniforms ──────────────────────────────────────────────────────────

void Shader::Bind() const { glUseProgram(id_); }

int Shader::UniformLocation(std::string_view name) const
{
    // GL caches uniform locations; repeated lookups are O(1) driver-side.
    return glGetUniformLocation(id_, name.data());
}

void Shader::SetInt    (std::string_view n, int v)              const { glUniform1i (UniformLocation(n), v); }
void Shader::SetFloat  (std::string_view n, float v)            const { glUniform1f (UniformLocation(n), v); }
void Shader::SetVec3   (std::string_view n, glm::vec3 v)        const { glUniform3fv(UniformLocation(n), 1, glm::value_ptr(v)); }
void Shader::SetMat4   (std::string_view n, const glm::mat4& m) const { glUniformMatrix4fv(UniformLocation(n), 1, GL_FALSE, glm::value_ptr(m)); }
void Shader::SetTexture(std::string_view n, int unit)           const { glUniform1i (UniformLocation(n), unit); }

// ─── Private helpers ──────────────────────────────────────────────────────────

std::uint32_t Shader::CompileStage(std::uint32_t glType, const std::string& src) const
{
    const std::uint32_t obj = glCreateShader(glType);
    const char* srcPtr = src.c_str();
    glShaderSource(obj, 1, &srcPtr, nullptr);
    glCompileShader(obj);

    int success = 0;
    glGetShaderiv(obj, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetShaderInfoLog(obj, sizeof(log), nullptr, log);
        LOG_ERROR("Shader compile error:\n{}", log);
        glDeleteShader(obj);
        return 0;
    }
    return obj;
}

std::uint32_t Shader::LinkProgram(std::uint32_t vert, std::uint32_t frag,
                                   std::uint32_t geom)
{
    const std::uint32_t prog = glCreateProgram();
    glAttachShader(prog, vert);
    glAttachShader(prog, frag);
    if (geom) glAttachShader(prog, geom);
    glLinkProgram(prog);

    int success = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetProgramInfoLog(prog, sizeof(log), nullptr, log);
        LOG_ERROR("Shader link error:\n{}", log);
        glDeleteProgram(prog);
        return 0;
    }

    // Explicitly bind the standard UBO blocks to their expected binding points.
    // This is idempotent with GLSL layout(binding = N) and acts as a fallback
    // on OpenGL 4.1 (macOS) where the explicit binding syntax requires
    // GL_ARB_shading_language_420pack — even if the GLSL directive was silently
    // ignored, the programmatic binding still takes effect.
    static constexpr struct { const char* name; GLuint point; } kBlocks[] = {
        {"PerFrameData",  0},
        {"PerObjectData", 1},
        {"ShadowData",    2},
    };
    for (const auto& b : kBlocks) {
        const GLuint idx = glGetUniformBlockIndex(prog, b.name);
        if (idx != GL_INVALID_INDEX)
            glUniformBlockBinding(prog, idx, b.point);
    }

    return prog;
}

} // namespace engine
