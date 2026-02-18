#pragma once

#include <renderer/frontend/RenderCommand.hpp>
#include <core/Geometry.hpp>
#include <vector>

namespace engine {

// ─── RenderQueue ──────────────────────────────────────────────────────────────
// Collects RenderCommands for a single frame, then sorts and exposes them
// to render passes.  Cleared at the start of each frame by the Renderer.
class RenderQueue {
public:
    void Submit(const RenderCommand& cmd);

    // Sort opaque front-to-back, transparents back-to-front, then append.
    void Sort();

    void Clear();

    // Separate views for shadow and geometry passes.
    // All shadow-casting opaques (transparents excluded from shadow).
    std::vector<RenderCommand> ShadowCasters() const;

    const std::vector<RenderCommand>& OpaqueCommands()      const { return opaques_; }
    const std::vector<RenderCommand>& TransparentCommands() const { return transparents_; }

    // Accumulated AABB of all submitted commands (updated on Submit).
    const AABB& SceneBounds() const { return sceneBounds_; }

    std::size_t TotalCount() const { return opaques_.size() + transparents_.size(); }

private:
    std::vector<RenderCommand> opaques_;
    std::vector<RenderCommand> transparents_;
    AABB                       sceneBounds_;
};

} // namespace engine
