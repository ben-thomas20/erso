#pragma once

#include <filesystem>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace engine {

// Resolves #include "relative/path" directives in GLSL source files.
//
// Rules:
//   - Paths are relative to the directory of the file that contains the directive.
//   - Each file is included at most once per compilation unit (cycle detection).
//   - After each #include, a #line directive is injected to restore line-number
//     context.  The directive uses integer source-string IDs (not filename strings)
//     because the string form (#line N "file") is only supported via the
//     GL_ARB_shading_language_include extension, which is not universally
//     available (notably absent on Apple's GL 4.1 / Metal driver).
//     A header comment block maps every integer ID to its canonical path so
//     GLSL error messages can be traced back to the correct file.
//
// Only the double-quoted form (#include "...") is supported; angle-bracket
// includes (#include <...>) are passed through unchanged.
// Result of processing a shader file: the resolved GLSL source plus the
// canonical paths of every file that was read (main + all transitive includes).
// Store `dependencies` to track timestamps for hot-reload.
struct ShaderProcessResult {
    std::string                          source;
    std::vector<std::filesystem::path>   dependencies;
};

class ShaderPreprocessor {
public:
    // Process a top-level shader file.
    // Returns the fully resolved source and the dependency file list.
    // Throws std::runtime_error on missing files or include cycles.
    static ShaderProcessResult Process(const std::filesystem::path& filePath);

private:
    // Assigns a stable integer ID to each unique source file encountered.
    // Shared across the recursive calls for a single compilation unit.
    struct SourceRegistry {
        std::map<std::filesystem::path, int> ids;
        int next = 0;

        int GetOrCreate(const std::filesystem::path& canonical) {
            auto [it, inserted] = ids.try_emplace(canonical, next);
            if (inserted) ++next;
            return it->second;
        }
    };

    static std::string ProcessImpl(const std::filesystem::path& filePath,
                                   std::set<std::filesystem::path>& visited,
                                   SourceRegistry&                  registry,
                                   bool                             emitLineDirective);
};

} // namespace engine
