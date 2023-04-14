#pragma once
#include <EASTL/array.h>
#include <EASTL/atomic.h>
#include <EASTL/type_traits.h>
#include "Common.hpp"

namespace DT::Primitive
{

template <typename T, uint32_t Count, bool BeCacheLineAlign = true>
class MPMC_FixedQueue final
{
    static_assert(Count > 0);
    static_assert(!(Count & (Count - 1)), "count must power of two");

    static constexpr uint32_t TypeAlignment = alignof(T) > sizeof(void*) ? alignof(T) : sizeof(void*);
    static constexpr uint32_t ElementAlignment =
        BeCacheLineAlign && ARC_CACHE_LINE_SIZE > TypeAlignment ? ARC_CACHE_LINE_SIZE : TypeAlignment;

    static constexpr uint32_t IndexMask = Count - 1;
    static constexpr uint32_t ReadableBit = (uint32_t)1 << 31;
    static constexpr uint32_t WriteableMask = ~ReadableBit;
    static constexpr uint32_t WriteableCheckSum(uint32_t index) noexcept { return index & WriteableMask; }
    static constexpr uint32_t ReadableCheckSum(uint32_t index) noexcept { return index | ReadableBit; }

    constexpr uint32_t WriteableCheckSumNextGen(uint32_t index) const noexcept { return (index + Count) & WriteableMask; }
    constexpr uint32_t ReadableCheckSumPrevGen(uint32_t index) const noexcept { return (index - Count) | ReadableBit; }
    constexpr uint32_t SlotIndex(uint32_t index) const { return index & IndexMask; }

    struct alignas(ElementAlignment) Slot
    {
        T value;
        eastl::atomic<uint32_t> check_sum;
    };

    Slot buffer[Count];
    alignas(ARC_CACHE_LINE_SIZE) eastl::atomic<uint32_t> read_pos;
    alignas(ARC_CACHE_LINE_SIZE) eastl::atomic<uint32_t> write_pos;

   public:
    MPMC_FixedQueue() noexcept;
    ~MPMC_FixedQueue() noexcept;

    bool TryPopFront(T& value) noexcept;

    bool TryPushBack(const T& value) noexcept;

    bool TryPushBack(T&& value) noexcept;

    static constexpr size_t Capacity() noexcept;

    static constexpr size_t BufferSize() noexcept;

    static constexpr size_t BufferAlignment() noexcept;

    template <typename... ARGS>
    bool TryEmplaceBack(ARGS&&... args) noexcept
    {
        while (true)
            {
                uint32_t pos = write_pos.load(eastl::memory_order_relaxed);
                Slot* slot = &buffer[SlotIndex(pos)];
                uint32_t check_sum = slot->check_sum.load(eastl::memory_order_acquire);

                // check if can write to pos
                while (check_sum == WriteableCheckSum(pos))
                    {
                        // try to update write pos
                        if (write_pos.compare_exchange_weak(pos, pos + 1, eastl::memory_order_relaxed, eastl::memory_order_relaxed))
                            {
                                // success , we snatch this pos value from other comsumers
                                // construct value in place
                                new (&slot->value) T(eastl::forward<ARGS>(args)...);
                                slot->check_sum.store(ReadableCheckSum(pos), eastl::memory_order_release);
                                return true;
                            }
                        else
                            {
                                // update slot ,try again
                                // pos is add by compare_exchange_weak
                                slot = &buffer[SlotIndex(pos)];
                                check_sum = slot->check_sum.load(eastl::memory_order_acquire);
                            }
                    }

                // reach a read alot that mean the queue full, SlotIndex(write_pos) == SlotIndex(read_pos)
                // write_pos - read_pos = count
                if (check_sum == ReadableCheckSumPrevGen(pos))
                    {
                        return false;
                    }
            }
    }
};

template <typename T, uint32_t Count, bool BeCacheLineAlign>
MPMC_FixedQueue<T, Count, BeCacheLineAlign>::MPMC_FixedQueue() noexcept
    : buffer{},
      read_pos{0},
      write_pos{0}
{
    for (uint32_t index = 0; index < Count; ++index)
        {
            buffer[index].check_sum.store(WriteableCheckSum(index), eastl::memory_order_relaxed);
        }

    eastl::atomic_thread_fence(eastl::memory_order_seq_cst);
}

template <typename T, uint32_t Count, bool BeCacheLineAlign>
MPMC_FixedQueue<T, Count, BeCacheLineAlign>::~MPMC_FixedQueue() noexcept
{
    for (;;)
        {
            const uint32_t pos = read_pos.fetch_add(1, eastl::memory_order_relaxed);
            Slot& slot = buffer[SlotIndex(pos)];
            if (slot.check_sum.load(eastl::memory_order_acquire) != ReadableCheckSum(pos))
                {
                    break;
                }

            Destruct(slot.value);
        }
}

template <typename T, uint32_t Count, bool BeCacheLineAlign>
bool MPMC_FixedQueue<T, Count, BeCacheLineAlign>::TryPopFront(T& value) noexcept
{
    while (true)
        // maybe no need while(true) here
        // if check_sum ==  WriteableCheckSum(pos), the queue is empty and break
        {
            uint32_t pos = read_pos.load(eastl::memory_order_relaxed);
            Slot* slot = &buffer[SlotIndex(pos)];
            uint32_t check_sum = slot->check_sum.load(eastl::memory_order_acquire);

            // check if can read from pos
            while (check_sum == ReadableCheckSum(pos))
                {
                    // try update read_pos
                    if (read_pos.compare_exchange_weak(pos, pos + 1, eastl::memory_order_relaxed, eastl::memory_order_relaxed))
                        {
                            // success , we snatch this pos value from other comsumers
                            value = std::move(slot->value);
                            Destruct(slot->value);
                            slot->check_sum.store(WriteableCheckSumNextGen(pos), eastl::memory_order_release);
                            return true;
                        }
                    else
                        {
                            // update slot ,try again
                            // pos is add by compare_exchange_weak
                            slot = &slot[SlotIndex(pos)];
                            check_sum = slot->check_sum.load(eastl::memory_order_acquire);
                        }
                }

            // read_pos reach write pos empty queue
            if (check_sum == WriteableCheckSum(pos))
                {
                    return false;
                }
        }
}

template <typename T, uint32_t Count, bool BeCacheLineAlign>
bool MPMC_FixedQueue<T, Count, BeCacheLineAlign>::TryPushBack(const T& value) noexcept
{
    return TryEmplaceBack(value);
}

template <typename T, uint32_t Count, bool BeCacheLineAlign>
bool MPMC_FixedQueue<T, Count, BeCacheLineAlign>::TryPushBack(T&& value) noexcept
{
    return TryEmplaceBack(eastl::forward<T>(value));
}

template <typename T, uint32_t Count, bool BeCacheLineAlign>
constexpr size_t MPMC_FixedQueue<T, Count, BeCacheLineAlign>::Capacity() noexcept
{
    return Count;
}

template <typename T, uint32_t Count, bool BeCacheLineAlign>
constexpr size_t MPMC_FixedQueue<T, Count, BeCacheLineAlign>::BufferSize() noexcept
{
    return sizeof(T) * Count;
}

template <typename T, uint32_t Count, bool BeCacheLineAlign>
constexpr size_t MPMC_FixedQueue<T, Count, BeCacheLineAlign>::BufferAlignment() noexcept
{
    return ElementAlignment;
}

}  // namespace DT::Primitive