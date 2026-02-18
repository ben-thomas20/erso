#include "ShaderPreprocessor.hpp"

#include <core/FileSystem.hpp>

#include <format>
#include <sstream>
#include <stdexcept>

namespace engine {

// ─── Public entry point ───────────────────────────────────────────────────────

ShaderProcessResult ShaderPreprocessor::Process(const std::filesystem::path& filePath)
{
    std::set<std::filesystem::path> visited;
    SourceRegistry                  registry;

    const std::string body =
        ProcessImpl(std::filesystem::canonical(filePath), visited, registry,
                    /*emitLineDirective=*/false);

    // Prepend a comment block mapping integer source IDs to canonical paths.
    std::ostringstream header;
    header << "// === ShaderPreprocessor source map ===\n";
    for (const auto& [path, id] : registry.ids)
        header << "// source " << id << ": " << path.string() << "\n";
    header << "// ======================================\n";

    // Collect dependency list from the visited set (includes the root file).
    std::vector<std::filesystem::path> deps(visited.begin(), visited.end());

    return { header.str() + body, std::move(deps) };
}

// ─── Recursive implementation ─────────────────────────────────────────────────

std::string ShaderPreprocessor::ProcessImpl(const std::filesystem::path& filePath,
                                            std::set<std::filesystem::path>& visited,
                                            SourceRegistry&                  registry,
                                            bool                             emitLineDirective)
{
    const std::filesystem::path canonical = std::filesystem::canonical(filePath);

    if (visited.contains(canonical)) {
        throw std::runtime_error(
            std::format("ShaderPreprocessor: include cycle detected for '{}'",
                        canonical.string()));
    }
    visited.insert(canonical);

    const auto source = fs::ReadFile(canonical);
    if (!source) {
        throw std::runtime_error(
            std::format("ShaderPreprocessor: cannot read file '{}'",
                        canonical.string()));
    }

    const int srcId = registry.GetOrCreate(canonical);
    const std::filesystem::path dir = canonical.parent_path();
    std::ostringstream out;

    // For included files only: emit #line to set driver-side source context.
    // Top-level files suppress this so #version remains the very first statement.
    // Standard GLSL #line syntax: #line line_number [source_string_number]
    if (emitLineDirective)
        out << "#line 1 " << srcId << "\n";

    int lineNumber = 1;
    std::istringstream in(*source);
    std::string line;

    while (std::getline(in, line)) {
        // Trim leading whitespace to identify directives.
        std::size_t i = 0;
        while (i < line.size() && (line[i] == ' ' || line[i] == '\t')) ++i;
        const std::string_view trimmed(line.data() + i, line.size() - i);

        if (trimmed.starts_with("#include")) {
            // Parse  →  #include "relative/path.glsl"
            const std::size_t open  = line.find('"');
            const std::size_t close = line.rfind('"');

            if (open == std::string::npos || open == close) {
                throw std::runtime_error(
                    std::format("ShaderPreprocessor: malformed #include at line {} in '{}'",
                                lineNumber, canonical.string()));
            }

            const std::string           rel  = line.substr(open + 1, close - open - 1);
            const std::filesystem::path full = std::filesystem::canonical(dir / rel);

            // Recurse — included files always receive a #line preamble.
            out << ProcessImpl(full, visited, registry, /*emitLineDirective=*/true);

            // Restore line context in the parent file.
            out << "#line " << (lineNumber + 1) << " " << srcId << "\n";
        } else {
            out << line << '\n';
        }

        ++lineNumber;
    }

    return out.str();
}

} // namespace engine
