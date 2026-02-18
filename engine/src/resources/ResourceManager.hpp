#pragma once

#include <core/Memory/HandlePool.hpp>
#include <resources/GPUMesh.hpp>
#include <resources/Material.hpp>
#include <resources/MeshBuffer.hpp>
#include <renderer/backend/Texture.hpp>
#include <renderer/backend/Shader.hpp>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace engine {

// ─── Handle types ─────────────────────────────────────────────────────────────
struct MeshTag    {};
struct TextureTag {};
using MeshHandle    = Handle<MeshTag>;
using TextureHandle = Handle<TextureTag>;

// ─── ResourceManager ──────────────────────────────────────────────────────────
// Central asset registry.
//
//  • All static mesh geometry shares a single MeshBuffer (mega VBO + IBO + VAO).
//  • Textures and materials are stored in typed HandlePools.
//  • Shader hot-reload: call TrackShaderForReload() for each Shader you want to
//    monitor, then call PollShaderReload() once per frame.
//  • Path-based caching: LoadMesh / LoadTexture return the cached handle when
//    the same canonical path is requested more than once.
class ResourceManager {
public:
    ResourceManager();   // creates default textures, pre-allocates MeshBuffer

    // ── Mesh ──────────────────────────────────────────────────────────────────

    // Upload CPU-side geometry to the shared MeshBuffer and cache by path.
    // Requesting the same path twice returns the cached handle.
    MeshHandle LoadMesh(const std::filesystem::path& path);

    // Load all meshes from a file and return their handles.
    std::vector<MeshHandle> LoadAllMeshes(const std::filesystem::path& path);

    // Upload a RawMesh not tied to a file (procedural / in-memory geometry).
    MeshHandle AddMesh(RawMesh mesh);

    const GPUMesh& GetMesh(MeshHandle handle) const;

    // ── Texture ───────────────────────────────────────────────────────────────

    TextureHandle LoadTexture(const std::filesystem::path& path,
                              bool sRGB       = true,
                              bool genMipmaps = true);

    const Texture& GetTexture(TextureHandle handle) const;

    // ── Material ──────────────────────────────────────────────────────────────

    MaterialHandle CreateMaterial(const Material& mat);
    const Material& GetMaterial(MaterialHandle handle) const;

    // Default 1×1 fallback textures (always valid after construction).
    const Texture& DefaultAlbedo()     const;
    const Texture& DefaultNormal()     const;
    const Texture& DefaultMetalRough() const;

    // ── Shader hot-reload ─────────────────────────────────────────────────────

    // Register a shader for hot-reload tracking.
    // Stores the current file_time_type for every dependency the shader reports.
    void TrackShaderForReload(Shader& shader);

    // Call once per frame. Checks whether any tracked shader source file has
    // changed on disk. On change, calls Shader::Reload(). On compile failure,
    // logs the error and keeps the previous program — engine never crashes.
    void PollShaderReload();

    // Name of the last successfully reloaded shader (filename only, e.g. "gbuffer.frag").
    // Empty string if no reload has occurred yet.
    const std::string& LastReloadedShader() const { return lastReloadedShader_; }

private:
    // ── Mesh mega-buffer ──────────────────────────────────────────────────────
    MeshBuffer meshBuffer_;

    HandlePool<GPUMesh,   MeshTag>     meshPool_;
    HandlePool<Texture,   TextureTag>  texturePool_;
    HandlePool<Material,  MaterialTag> materialPool_;

    std::unordered_map<std::string, MeshHandle>    meshCache_;
    std::unordered_map<std::string, TextureHandle> textureCache_;

    // ── Default textures ──────────────────────────────────────────────────────
    Texture defaultAlbedo_;
    Texture defaultNormal_;
    Texture defaultMetalRough_;

    // ── Shader hot-reload tracking ────────────────────────────────────────────
    struct ShaderRecord {
        Shader*                                          shader;
        std::vector<std::filesystem::path>               deps;
        std::vector<std::filesystem::file_time_type>     timestamps;
    };
    std::vector<ShaderRecord> trackedShaders_;
    std::string               lastReloadedShader_;

    // Re-read current timestamps for all deps of a record.
    void RefreshTimestamps(ShaderRecord& rec);
};

} // namespace engine
