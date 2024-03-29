#include "../interface/Time.hpp"

namespace DT::Primitive
{

Timer::Timer() noexcept { Reset(); }

void Timer::Reset() { start_time = std::chrono::high_resolution_clock().now(); }

double Timer::ElapsedTime()
{
    const auto& now = std::chrono::high_resolution_clock().now();
    const auto& time_span = std::chrono::duration_cast<std::chrono::duration<double>>(now - start_time);
    return time_span.count();
}

float Timer::ElapsedTimef()
{
    const auto& now = std::chrono::high_resolution_clock().now();
    const auto& time_span = std::chrono::duration_cast<std::chrono::duration<float>>(now - start_time);
    return time_span.count();
}
}  // namespace DT::Primitive