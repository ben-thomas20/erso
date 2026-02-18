#include "FileSystem.hpp"

#include <fstream>
#include <sstream>

namespace engine::fs {

std::optional<std::string> ReadFile(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file.is_open()) return std::nullopt;

    std::ostringstream ss;
    ss << file.rdbuf();
    if (file.bad()) return std::nullopt;

    return ss.str();
}

bool WriteFile(const std::filesystem::path& path, std::string_view content)
{
    std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!file.is_open()) return false;

    file.write(content.data(), static_cast<std::streamsize>(content.size()));
    return !file.bad();
}

bool Exists(const std::filesystem::path& path)
{
    return std::filesystem::exists(path);
}

} // namespace engine::fs
