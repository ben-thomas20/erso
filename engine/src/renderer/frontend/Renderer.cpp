#include <renderer/frontend/Renderer.hpp>

namespace engine {

// ─── UniformBufferCache ───────────────────────────────────────────────────────

template<typename T>
void UniformBufferCache::Upload(std::optional<Buffer>& slot,
                                std::uint32_t          bindingPoint,
                                const T&               data)
{
    if (!slot.has_value())
        slot.emplace(BufferTarget::Uniform, BufferUsage::DynamicDraw, sizeof(T));

    slot->Upload(0, sizeof(T), &data);
    slot->BindBase(bindingPoint);
}

void UniformBufferCache::UploadPerFrame (const PerFrameData&  d) { Upload(perFrameUBO_,  0, d); }
void UniformBufferCache::UploadPerObject(const PerObjectData& d) { Upload(perObjectUBO_, 1, d); }
void UniformBufferCache::UploadShadow   (const ShadowData&    d) { Upload(shadowUBO_,    2, d); }

// ─── Renderer ─────────────────────────────────────────────────────────────────

Renderer::Renderer(std::uint32_t w, std::uint32_t h)
    : shadowPass_ ()
    , geoPass_    (w, h)
    , lightingPass_(w, h)
    , postPass_   (w, h)
{}

void Renderer::Resize(std::uint32_t w, std::uint32_t h)
{
    geoPass_.OnResize(w, h);
    lightingPass_.OnResize(w, h);
    postPass_.OnResize(w, h);
}

void Renderer::Submit(const RenderCommand& cmd)
{
    queue_.Submit(cmd);
}

const Texture& Renderer::RenderFrame(const FrameContext& ctx)
{
    // Collect GPU times from the previous frame.  Only update entries that
    // have a fresh result; stale entries persist so the overlay never flickers
    // to zero.  EMA (α=0.15) smooths the bar chart without hiding real spikes.
    const auto freshTimes = gpuTimer_.CollectResults();
    for (const auto& [label, ms] : freshTimes) {
        auto it = lastGPUTimes_.find(label);
        if (it == lastGPUTimes_.end())
            lastGPUTimes_[label] = ms;
        else
            it->second = it->second * 0.85f + ms * 0.15f;
    }

    queue_.Sort();

    ubos_.UploadPerFrame(ctx.frame);

    gpuTimer_.Begin("Shadow");
    shadowPass_.Execute(queue_, ctx.frame, ubos_,
                        ctx.lightDir, ctx.lightColor, ctx.lightIntensity);
    gpuTimer_.End("Shadow");

    gpuTimer_.Begin("GBuffer");
    geoPass_.Execute(queue_, ctx.frame, ubos_);
    gpuTimer_.End("GBuffer");

    gpuTimer_.Begin("Lighting");
    lightingPass_.Execute(geoPass_.Normal(),
                          geoPass_.Albedo(),
                          geoPass_.Material(),
                          geoPass_.Depth(),
                          shadowPass_.ShadowMap(),
                          ubos_);
    gpuTimer_.End("Lighting");

    gpuTimer_.Begin("PostFX");
    const Texture& output = postPass_.Execute(lightingPass_.HDROutput());
    gpuTimer_.End("PostFX");

    queue_.Clear();
    return output;
}

} // namespace engine
