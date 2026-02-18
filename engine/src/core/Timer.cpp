#include "Timer.hpp"

namespace engine {

Timer::Timer()
    : start_(Clock::now())
{
}

void Timer::Reset()
{
    start_ = Clock::now();
}

float Timer::ElapsedSeconds() const
{
    const auto elapsed = Clock::now() - start_;
    return std::chrono::duration<float>(elapsed).count();
}

float Timer::ElapsedMilliseconds() const
{
    const auto elapsed = Clock::now() - start_;
    return std::chrono::duration<float, std::milli>(elapsed).count();
}

} // namespace engine
