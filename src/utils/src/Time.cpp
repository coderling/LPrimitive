#include "Time.hpp"

namespace CDL::Primitive
{

Timer::Timer() noexcept { Reset(); }

void Timer::Reset() { start_time = eastl::chrono::high_resolution_clock().now(); }

double Timer::ElapsedTime()
{
    const auto& now = eastl::chrono::high_resolution_clock().now();
    const auto& time_span = eastl::chrono::duration_cast<eastl::chrono::duration<double>>(now - start_time);
    return time_span.count();
}

float Timer::ElapsedTimef()
{
    const auto& now = eastl::chrono::high_resolution_clock().now();
    const auto& time_span = eastl::chrono::duration_cast<eastl::chrono::duration<float>>(now - start_time);
    return time_span.count();
}
}  // namespace CDL::Primitive