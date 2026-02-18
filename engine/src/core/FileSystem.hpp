#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace engine::fs {

// Read the entire contents of a text file.
// Returns std::nullopt on failure (file not found, permission error, etc.).
std::optional<std::string> ReadFile(const std::filesystem::path& path);

// Write text content to a file, creating or truncating it.
// Returns false on failure.
bool WriteFile(const std::filesystem::path& path, std::string_view content);

// Check whether a path exists on disk.
bool Exists(const std::filesystem::path& path);

} // namespace engine::fs
