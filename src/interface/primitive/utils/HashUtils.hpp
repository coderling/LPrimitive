#pragma once
#include <EASTL/functional.h>

namespace CDL::Primitive
{
// boost hash_combine
template <typename T>
inline void HashCombine(size_t& hash, const T& v)
{
    hash ^= eastl::hash<T>{}(v) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
}

template <typename FirstArgsType, typename... RestArgsType>
inline void HashCombine(size_t& hash_va, const FirstArgsType& arg, const RestArgsType&... rest_args)
{
    HashCombine(hash_va, arg);
    HashCombine(hash_va, rest_args...);
}

template <typename... ArgsType>
inline size_t ComputeHash(const ArgsType&... args)
{
    size_t hash_va = 0;
    HashCombine(hash_va, args...);

    return hash_va;
}

template <typename Iter>
inline size_t HansRange(Iter begin, Iter end)
{
    size_t hash = 0;
    for (; begin != end; ++begin)
        {
            HashCombine(hash, *begin);
        }
    return hash;
}

inline size_t ComputeHash(const void* bytes, uint32_t size_in_bytes)
{
    const uint8_t* p = (uint8_t*)bytes;
    uint32_t result = 2166136261U;
    while (size_in_bytes--) result = (result * 16777619) ^ (*p++);

    return result;
}

inline void ComputeHashCombine(size_t& hash, const void* bytes, uint32_t size_in_bytes)
{
    const uint8_t* p = (uint8_t*)bytes;
    uint32_t result = (uint32_t)hash;
    while (size_in_bytes--) result = (result * 16777619) ^ (*p++);
}

}  // namespace CDL::Primitive