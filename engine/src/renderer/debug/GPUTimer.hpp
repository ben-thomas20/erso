#pragma once

#include <glad/gl.h>
#include <string>
#include <string_view>
#include <unordered_map>
#include <array>

namespace engine {

// ─── GPUTimer ─────────────────────────────────────────────────────────────────
// Double-buffered GPU pass timer using GL_TIME_ELAPSED queries.
//
// Because GPU queries are asynchronous, results from the *current* frame are
// not available yet.  CollectResults() reads the *previous* frame's data
// (one-frame latency) and then flips the internal frame index.
//
// Typical use per frame:
//   timer.Begin("Shadow");   shadowPass.Execute();   timer.End("Shadow");
//   timer.Begin("GBuffer");  geoPass.Execute();       timer.End("GBuffer");
//   ...
//   auto ms = timer.CollectResults();  // read last frame, flip buffer
class GPUTimer {
public:
    GPUTimer()  = default;
    ~GPUTimer();

    GPUTimer(const GPUTimer&)            = delete;
    GPUTimer& operator=(const GPUTimer&) = delete;
    GPUTimer(GPUTimer&&)                 = delete;
    GPUTimer& operator=(GPUTimer&&)      = delete;

    void Begin(std::string_view label);
    void End  (std::string_view label);

    // Flip the double-buffer and return the previous frame's results in
    // milliseconds.  Labels with no result yet (first two frames) are omitted.
    std::unordered_map<std::string, float> CollectResults();

private:
    struct QueryPair {
        std::array<GLuint, 2> ids   = {0u, 0u};
        std::array<bool,   2> valid = {false, false};
    };

    std::unordered_map<std::string, QueryPair> queries_;
    int currentFrame_ = 0;  // toggles 0/1 each CollectResults() call
};

} // namespace engine
