#pragma once
#include <atomic>
#include <concepts>

namespace DT::Primitive
{

template <typename T, typename VersionT = uint16_t>
    requires std::unsigned_integral<VersionT>
class ThreadSafeVersionPool
{
    struct VersionElement
    {
        std::atomic<VersionT> version;
        T value;
    };

    static constexpr std::size_t ElementSize = sizeof(VersionElement);
    static constexpr uint32_t VersionT_Bits = sizeof(VersionT) * 8;
    // is element is i pool the version last bit is 0
    static constexpr VersionT RecycleVersionMask = (VersionT)1 << (VersionT_Bits - 1);
};
}  // namespace DT::Primitive