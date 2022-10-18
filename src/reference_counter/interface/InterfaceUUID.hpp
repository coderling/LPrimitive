#pragma once
#include <stdint.h>
#include <vcruntime_string.h>
#include "HashUtils.hpp"
#include "Logger.hpp"

namespace CDL::Primitive
{
struct IUUID_DATA
{
    uint32_t d1;
    uint16_t d2;
    uint16_t d3;
    uint8_t array[8];

    bool operator==(const IUUID_DATA& rhs) const
    {
        return d1 == rhs.d1 && d2 == rhs.d2 && d3 == rhs.d3 && memcmp(array, rhs.array, sizeof(array)) == 0;
    }
};

struct IUUID
{
    const char* name = nullptr;
    IUUID_DATA data;
    bool operator==(const IUUID& rhs) const { return data == rhs.data; }
};

static constexpr const IUUID UUID_UNKNOWN = {"UUID_UNKNOWN", {0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}}};

}  // namespace CDL::Primitive

#define STR(x) #x

#define INTERFACEUUID_BEGIN(TYPE) static constexpr const IUUID TYPE##_UUID = {STR(TYPE##_UUID),

#define INTERFACEUUID_END }

namespace std
{
template <>
struct hash<CDL::Primitive::IUUID>
{
   public:
    size_t operator()(const CDL::Primitive::IUUID& uuid) const
    {
        size_t hash_va = 0;
        CDL::Primitive::ComputeHash(hash_va, uuid.data.d1, uuid.data.d3, uuid.data.d3);
        for (int i = 0; i < 8; ++i)
            {
                CDL::Primitive::HashCombine(hash_va, uuid.data.array[i]);
            }

        return hash_va;
    }
};
}  // namespace std
