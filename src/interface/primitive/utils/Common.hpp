#pragma once

#ifndef ARC_CACHE_LINE_SIZE
#define ARC_CACHE_LINE_SIZE 64
#endif

#include <EASTL/atomic.h>

namespace CDL::Primitive
{

template <typename T1, typename T2>
inline static T1* PtrAdd(T1* l, T2 add) noexcept
{
    return (T1*)(uintptr_t(l) + uintptr_t(add));
}

template <typename T1, typename T2>
inline static T1* PtrSub(T1* l, T2 sub) noexcept
{
    return (T1*)(uintptr_t(l) - uintptr_t(sub));
}

template <typename Type>
inline static uint32_t GetLSB(Type&& val)
{
    if (val == 0) return sizeof(Type) * 8;

    uint32_t lsb = 0;
    while (!(val & (1 << lsb)))
        {
            lsb++;
        }

    return lsb;
}

template <typename T>
typename eastl::enable_if<eastl::is_destructible<T>::value, void>::type Destruct(T& v)
{
    v.~T();
}

template <typename T>
typename eastl::enable_if<!eastl::is_destructible<T>::value, void>::type Destruct(T& v)
{
}

class MPMC_Node
{
   public:
    eastl::atomic<MPMC_Node*> next;
};

class MPSC_Node
{
   public:
    eastl::atomic<MPSC_Node*> next;
};
#define ENUM_OPERATORS(EnumClass)                                                                                                          \
    inline EnumClass& operator|=(EnumClass& lhs, EnumClass rhs)                                                                            \
    {                                                                                                                                      \
        return lhs = (EnumClass)((__underlying_type(EnumClass))rhs | (__underlying_type(EnumClass))rhs);                                   \
    }                                                                                                                                      \
    inline EnumClass& operator&=(EnumClass& lhs, EnumClass rhs)                                                                            \
    {                                                                                                                                      \
        return lhs = (EnumClass)((__underlying_type(EnumClass))rhs & (__underlying_type(EnumClass))rhs);                                   \
    }                                                                                                                                      \
    inline EnumClass& operator^=(EnumClass& lhs, EnumClass rhs)                                                                            \
    {                                                                                                                                      \
        return lhs = (EnumClass)((__underlying_type(EnumClass))rhs ^ (__underlying_type(EnumClass))rhs);                                   \
    }                                                                                                                                      \
    inline constexpr EnumClass operator|(EnumClass lhs, EnumClass rhs)                                                                     \
    {                                                                                                                                      \
        return (EnumClass)((__underlying_type(EnumClass))lhs | (__underlying_type(EnumClass))rhs);                                         \
    }                                                                                                                                      \
    inline constexpr EnumClass operator&(EnumClass lhs, EnumClass rhs)                                                                     \
    {                                                                                                                                      \
        return (EnumClass)((__underlying_type(EnumClass))lhs & (__underlying_type(EnumClass))rhs);                                         \
    }                                                                                                                                      \
    inline constexpr EnumClass operator^(EnumClass lhs, EnumClass rhs)                                                                     \
    {                                                                                                                                      \
        return (EnumClass)((__underlying_type(EnumClass))lhs ^ (__underlying_type(EnumClass))rhs);                                         \
    }                                                                                                                                      \
    inline constexpr bool operator!(EnumClass e)                                                                                           \
    {                                                                                                                                      \
        return !(__underlying_type(EnumClass))e;                                                                                           \
    }                                                                                                                                      \
    inline constexpr EnumClass operator~(EnumClass e)                                                                                      \
    {                                                                                                                                      \
        return (EnumClass) ~(__underlying_type(EnumClass))e;                                                                               \
    }                                                                                                                                      \
    inline bool HAS_ANY_FLAGS(EnumClass e, EnumClass flags)                                                                                \
    {                                                                                                                                      \
        return (e & flags) != (EnumClass)0;                                                                                                \
    }                                                                                                                                      \
    inline bool HAS_ANY_FLAGS(__underlying_type(EnumClass) e, EnumClass flags)                                                             \
    {                                                                                                                                      \
        return (e & (__underlying_type(EnumClass))flags) != 0;                                                                             \
    }

}  // namespace CDL::Primitive