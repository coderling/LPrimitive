#pragma once

// 内存模型解析：https://www.zhihu.com/question/24301047
// 内存模型：https://zhuanlan.zhihu.com/p/382372072
// ref:https://rigtorp.se/spinlock/
// ref:https://en.cppreference.com/w/cpp/atomic/memory_order

#include <EASTL/atomic.h>
#include "CommonDefines.hpp"

namespace CDL::Primitive
{
class Spinlock
{
    eastl::atomic<bool> is_locked{false};

   public:
    NOCOPY_AND_NOMOVE(Spinlock);

    Spinlock() noexcept {}

    void lock() noexcept
    {
        while (true)
            {
                //
                if (!is_locked.exchange(true, eastl::memory_order_acquire))
                    {
                        return;
                    }

                while (is_locked.load(eastl::memory_order_relaxed))
                    {
                        Pause();
                    }
            }
    }

    bool try_Lock() noexcept
    {
        return !is_locked.load(eastl::memory_order_relaxed) && !is_locked.exchange(true, eastl::memory_order_acquire);
    }

    void unlock() noexcept { is_locked.store(false, eastl::memory_order_release); }

   private:
    void Pause() noexcept;
};
}  // namespace CDL::Primitive