#include <resources/MeshLoader.hpp>
#include <core/Log.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

namespace engine {

// ─── Assimp helper ────────────────────────────────────────────────────────────

static RawMesh BuildRawMesh(const aiMesh* mesh)
{
    RawMesh raw;
    raw.vertices.reserve(mesh->mNumVertices);

    const bool hasUV      = mesh->HasTextureCoords(0);
    const bool hasTangent = mesh->HasTangentsAndBitangents();

    for (unsigned i = 0; i < mesh->mNumVertices; ++i) {
        MeshVertex v{};
        v.position = {mesh->mVertices[i].x,
                      mesh->mVertices[i].y,
                      mesh->mVertices[i].z};
        if (mesh->HasNormals()) {
            v.normal = {mesh->mNormals[i].x,
                        mesh->mNormals[i].y,
                        mesh->mNormals[i].z};
        }
        if (hasUV) {
            v.uv = glm::vec2{mesh->mTextureCoords[0][i].x,
                             mesh->mTextureCoords[0][i].y};
        }
        if (hasTangent) {
            v.tangent = {mesh->mTangents[i].x,
                         mesh->mTangents[i].y,
                         mesh->mTangents[i].z};
        }
        raw.localBounds.Expand(v.position);
        raw.vertices.push_back(v);
    }

    raw.indices.reserve(static_cast<std::size_t>(mesh->mNumFaces) * 3);
    for (unsigned f = 0; f < mesh->mNumFaces; ++f) {
        const aiFace& face = mesh->mFaces[f];
        for (unsigned j = 0; j < face.mNumIndices; ++j)
            raw.indices.push_back(face.mIndices[j]);
    }

    return raw;
}

std::vector<RawMesh> MeshLoader::Load(const std::filesystem::path& path)
{
    Assimp::Importer importer;
    const unsigned flags =
        aiProcess_Triangulate       |
        aiProcess_GenSmoothNormals  |
        aiProcess_CalcTangentSpace  |
        aiProcess_FlipUVs           |
        aiProcess_JoinIdenticalVertices;

    const aiScene* scene = importer.ReadFile(path.string(), flags);
    if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode) {
        LOG_ERROR("MeshLoader: {}", importer.GetErrorString());
        return {};
    }

    std::vector<RawMesh> result;
    result.reserve(scene->mNumMeshes);
    for (unsigned i = 0; i < scene->mNumMeshes; ++i)
        result.push_back(BuildRawMesh(scene->mMeshes[i]));

    LOG_INFO("MeshLoader: loaded {} mesh(es) from '{}'",
             result.size(), path.string());
    return result;
}

// ─── Procedural box ───────────────────────────────────────────────────────────

static MeshVertex MV(float px, float py, float pz,
                     float nx, float ny, float nz,
                     float u,  float v,
                     float tx, float ty, float tz)
{
    MeshVertex m{};
    m.position = {px, py, pz};
    m.normal   = {nx, ny, nz};
    m.uv       = glm::vec2{u, v};
    m.tangent  = {tx, ty, tz};
    return m;
}

static std::vector<MeshVertex> MakeBoxVertices(float n)
{
    std::vector<MeshVertex> verts;
    verts.reserve(24);
    // +Z front
    verts.push_back(MV(-n,-n, n,  0, 0, 1,  0,0,  1,0,0));
    verts.push_back(MV( n,-n, n,  0, 0, 1,  1,0,  1,0,0));
    verts.push_back(MV( n, n, n,  0, 0, 1,  1,1,  1,0,0));
    verts.push_back(MV(-n, n, n,  0, 0, 1,  0,1,  1,0,0));
    // -Z back
    verts.push_back(MV( n,-n,-n,  0, 0,-1,  0,0, -1,0,0));
    verts.push_back(MV(-n,-n,-n,  0, 0,-1,  1,0, -1,0,0));
    verts.push_back(MV(-n, n,-n,  0, 0,-1,  1,1, -1,0,0));
    verts.push_back(MV( n, n,-n,  0, 0,-1,  0,1, -1,0,0));
    // -X left
    verts.push_back(MV(-n,-n,-n, -1, 0, 0,  0,0,  0,0,1));
    verts.push_back(MV(-n,-n, n, -1, 0, 0,  1,0,  0,0,1));
    verts.push_back(MV(-n, n, n, -1, 0, 0,  1,1,  0,0,1));
    verts.push_back(MV(-n, n,-n, -1, 0, 0,  0,1,  0,0,1));
    // +X right
    verts.push_back(MV( n,-n, n,  1, 0, 0,  0,0,  0,0,-1));
    verts.push_back(MV( n,-n,-n,  1, 0, 0,  1,0,  0,0,-1));
    verts.push_back(MV( n, n,-n,  1, 0, 0,  1,1,  0,0,-1));
    verts.push_back(MV( n, n, n,  1, 0, 0,  0,1,  0,0,-1));
    // +Y top
    verts.push_back(MV(-n, n, n,  0, 1, 0,  0,0,  1,0,0));
    verts.push_back(MV( n, n, n,  0, 1, 0,  1,0,  1,0,0));
    verts.push_back(MV( n, n,-n,  0, 1, 0,  1,1,  1,0,0));
    verts.push_back(MV(-n, n,-n,  0, 1, 0,  0,1,  1,0,0));
    // -Y bottom
    verts.push_back(MV(-n,-n,-n,  0,-1, 0,  0,0,  1,0,0));
    verts.push_back(MV( n,-n,-n,  0,-1, 0,  1,0,  1,0,0));
    verts.push_back(MV( n,-n, n,  0,-1, 0,  1,1,  1,0,0));
    verts.push_back(MV(-n,-n, n,  0,-1, 0,  0,1,  1,0,0));
    return verts;
}

RawMesh MeshLoader::CreateBox(float halfExtent)
{
    RawMesh raw;
    raw.vertices = MakeBoxVertices(halfExtent);

    raw.indices.reserve(36);
    for (std::uint32_t face = 0; face < 6; ++face) {
        const std::uint32_t b = face * 4;
        raw.indices.insert(raw.indices.end(), {b, b+1, b+2, b, b+2, b+3});
    }

    raw.localBounds.min = glm::vec3(-halfExtent);
    raw.localBounds.max = glm::vec3( halfExtent);
    return raw;
}

} // namespace engine
