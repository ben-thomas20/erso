#include <renderer/debug/GPUTimer.hpp>

namespace engine {

GPUTimer::~GPUTimer()
{
    for (auto& [label, qp] : queries_) {
        for (GLuint id : qp.ids) {
            if (id) glDeleteQueries(1, &id);
        }
    }
}

void GPUTimer::Begin(std::string_view label)
{
    const std::string key(label);
    auto& qp = queries_[key];
    if (qp.ids[currentFrame_] == 0)
        glGenQueries(1, &qp.ids[currentFrame_]);

    glBeginQuery(GL_TIME_ELAPSED, qp.ids[currentFrame_]);
}

void GPUTimer::End(std::string_view label)
{
    glEndQuery(GL_TIME_ELAPSED);
    queries_[std::string(label)].valid[currentFrame_] = true;
}

std::unordered_map<std::string, float> GPUTimer::CollectResults()
{
    const int readFrame = currentFrame_;
    currentFrame_ = 1 - currentFrame_;

    std::unordered_map<std::string, float> results;
    for (auto& [label, qp] : queries_) {
        if (!qp.valid[readFrame] || qp.ids[readFrame] == 0) continue;

        GLint available = 0;
        glGetQueryObjectiv(qp.ids[readFrame], GL_QUERY_RESULT_AVAILABLE, &available);
        if (!available) continue;

        GLuint64 elapsed = 0;
        glGetQueryObjectui64v(qp.ids[readFrame], GL_QUERY_RESULT, &elapsed);
        results[label] = static_cast<float>(elapsed) / 1.0e6f;  // nanoseconds → milliseconds
    }
    return results;
}

} // namespace engine
