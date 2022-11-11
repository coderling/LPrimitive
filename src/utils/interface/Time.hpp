#pragma once

#include <EASTL/chrono.h>

namespace CDL::Primitive
{
class Timer final
{
   public:
    Timer() noexcept;
    void Reset();
    double ElapsedTime();
    float ElapsedTimef();

   private:
    eastl::chrono::high_resolution_clock::time_point start_time;
};
}  // namespace CDL::Primitive