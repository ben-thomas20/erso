#include <renderer/frontend/RenderQueue.hpp>
#include <algorithm>

namespace engine {

void RenderQueue::Submit(const RenderCommand& cmd)
{
    if (cmd.transparent)
        transparents_.push_back(cmd);
    else
        opaques_.push_back(cmd);

    // Expand scene AABB using the mesh's transformed origin as an approximation.
    // A tighter fit (transforming the local AABB) is done by Phase 6 culling.
    const glm::vec3 origin = glm::vec3(cmd.modelMatrix[3]);
    sceneBounds_.Expand(origin);
}

void RenderQueue::Sort()
{
    // Opaques: front-to-back (minimise overdraw)
    std::sort(opaques_.begin(), opaques_.end(),
              [](const RenderCommand& a, const RenderCommand& b) {
                  return a.distanceToCamera < b.distanceToCamera;
              });

    // Transparents: back-to-front (correct alpha blending)
    std::sort(transparents_.begin(), transparents_.end(),
              [](const RenderCommand& a, const RenderCommand& b) {
                  return a.distanceToCamera > b.distanceToCamera;
              });
}

void RenderQueue::Clear()
{
    opaques_.clear();
    transparents_.clear();
    sceneBounds_ = AABB{};
}

std::vector<RenderCommand> RenderQueue::ShadowCasters() const
{
    std::vector<RenderCommand> result;
    result.reserve(opaques_.size());
    for (const auto& cmd : opaques_)
        if (cmd.castsShadow) result.push_back(cmd);
    return result;
}

} // namespace engine
