#pragma once

#include <resources/GPUMesh.hpp>
#include <filesystem>
#include <vector>

namespace engine {

class MeshLoader {
public:
    // Load all meshes from a file (GLTF, OBJ, FBX, …) using Assimp.
    // Returns one RawMesh per aiMesh node.
    // Returns an empty vector on failure (error is logged).
    static std::vector<RawMesh> Load(const std::filesystem::path& path);

    // Generate a unit box centred at the origin as CPU-side RawMesh data.
    static RawMesh CreateBox(float halfExtent = 0.5f);
};

} // namespace engine
