#include <resources/ResourceManager.hpp>
#include <resources/MeshLoader.hpp>
#include <core/Log.hpp>

#include <array>
#include <cstdint>
#include <filesystem>
#include <system_error>

namespace engine {

// ─── Constructor ──────────────────────────────────────────────────────────────

ResourceManager::ResourceManager()
{
    // Default 1×1 fallback textures for materials that have no texture assigned.
    const std::array<std::uint8_t, 4> white      = {255, 255, 255, 255};
    const std::array<std::uint8_t, 4> flatNormal  = {128, 128, 255, 255};
    const std::array<std::uint8_t, 4> orm         = {255, 128,   0, 255}; // AO=1, rough=0.5, metal=0

    defaultAlbedo_     = Texture::FromData(1, 1, TextureFormat::RGBA8, white.data(),     false);
    defaultNormal_     = Texture::FromData(1, 1, TextureFormat::RGBA8, flatNormal.data(),false);
    defaultMetalRough_ = Texture::FromData(1, 1, TextureFormat::RGBA8, orm.data(),       false);

    LOG_INFO("ResourceManager: created default fallback textures");
}

// ─── Default texture accessors ────────────────────────────────────────────────

const Texture& ResourceManager::DefaultAlbedo()     const { return defaultAlbedo_;     }
const Texture& ResourceManager::DefaultNormal()     const { return defaultNormal_;     }
const Texture& ResourceManager::DefaultMetalRough() const { return defaultMetalRough_; }

// ─── Mesh ─────────────────────────────────────────────────────────────────────

MeshHandle ResourceManager::AddMesh(RawMesh raw)
{
    const auto alloc = meshBuffer_.Upload(raw.vertices, raw.indices);

    GPUMesh gpu;
    gpu.sharedVAOID = meshBuffer_.GetVAO();
    gpu.baseVertex  = alloc.baseVertex;
    gpu.baseIndex   = alloc.baseIndex;
    gpu.indexCount  = static_cast<std::uint32_t>(raw.indices.size());
    gpu.localBounds = raw.localBounds;

    return meshPool_.Insert(std::move(gpu));
}

MeshHandle ResourceManager::LoadMesh(const std::filesystem::path& path)
{
    const std::string key = std::filesystem::weakly_canonical(path).string();
    auto it = meshCache_.find(key);
    if (it != meshCache_.end()) return it->second;

    const auto raws = MeshLoader::Load(path);
    if (raws.empty()) return MeshHandle{};

    // Cache only the first mesh; use LoadAllMeshes for multi-mesh files.
    MeshHandle h = AddMesh(raws.front());
    meshCache_.emplace(key, h);
    return h;
}

std::vector<MeshHandle> ResourceManager::LoadAllMeshes(const std::filesystem::path& path)
{
    const auto raws = MeshLoader::Load(path);
    std::vector<MeshHandle> handles;
    handles.reserve(raws.size());
    for (const auto& raw : raws)
        handles.push_back(AddMesh(raw));
    return handles;
}

const GPUMesh& ResourceManager::GetMesh(MeshHandle handle) const
{
    return meshPool_.Get(handle);
}

// ─── Texture ─────────────────────────────────────────────────────────────────

TextureHandle ResourceManager::LoadTexture(const std::filesystem::path& path,
                                            bool /*sRGB*/, bool genMipmaps)
{
    const std::string key = std::filesystem::weakly_canonical(path).string();
    auto it = textureCache_.find(key);
    if (it != textureCache_.end()) return it->second;

    Texture tex = Texture::FromFile(path, genMipmaps);
    if (!tex.IsValid()) return TextureHandle{};

    TextureHandle h = texturePool_.Insert(std::move(tex));
    textureCache_.emplace(key, h);
    return h;
}

const Texture& ResourceManager::GetTexture(TextureHandle handle) const
{
    return texturePool_.Get(handle);
}

// ─── Material ─────────────────────────────────────────────────────────────────

MaterialHandle ResourceManager::CreateMaterial(const Material& mat)
{
    return materialPool_.Insert(mat);
}

const Material& ResourceManager::GetMaterial(MaterialHandle handle) const
{
    return materialPool_.Get(handle);
}

// ─── Shader hot-reload ────────────────────────────────────────────────────────

void ResourceManager::RefreshTimestamps(ShaderRecord& rec)
{
    rec.timestamps.clear();
    rec.timestamps.reserve(rec.deps.size());
    for (const auto& p : rec.deps) {
        std::error_code ec;
        rec.timestamps.push_back(std::filesystem::last_write_time(p, ec));
    }
}

void ResourceManager::TrackShaderForReload(Shader& shader)
{
    ShaderRecord rec;
    rec.shader = &shader;
    rec.deps   = shader.GetDependencies();  // captured after first successful compile
    RefreshTimestamps(rec);
    trackedShaders_.push_back(std::move(rec));
}

void ResourceManager::PollShaderReload()
{
    for (auto& rec : trackedShaders_) {
        bool changed = false;

        // Check whether any dependency has a newer modification time.
        for (std::size_t i = 0; i < rec.deps.size(); ++i) {
            std::error_code ec;
            const auto newTime = std::filesystem::last_write_time(rec.deps[i], ec);
            if (!ec && newTime != rec.timestamps[i]) {
                changed = true;
                break;
            }
        }

        if (!changed) continue;

        LOG_INFO("ResourceManager: shader source changed — reloading '{}'",
                 rec.deps.empty() ? "?" : rec.deps.front().filename().string());

        if (rec.shader->Reload()) {
            rec.deps = rec.shader->GetDependencies();
            RefreshTimestamps(rec);
            // Record the name of the changed file for the ImGui indicator.
            if (!rec.deps.empty())
                lastReloadedShader_ = rec.deps.front().filename().string();
        } else {
            LOG_WARN("ResourceManager: shader reload failed — keeping previous program");
        }
    }
}

} // namespace engine
