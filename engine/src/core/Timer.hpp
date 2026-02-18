#pragma once

#include <chrono>

namespace engine {

class Timer {
public:
    Timer();

    // Reset the start point to now.
    void Reset();

    // Elapsed time since construction or last Reset().
    float ElapsedSeconds() const;
    float ElapsedMilliseconds() const;

private:
    using Clock     = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    TimePoint start_;
};

} // namespace engine
