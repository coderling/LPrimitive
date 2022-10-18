#pragma once
#include <stdint.h>

namespace CDL::Primitive
{
struct Misc
{
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

    template <typename T1, typename T2>
    inline static T1* PtrAdd(T1* l, T2 add) noexcept
    {
        return (T1*)(uintptr_t(l) + uintptr_t(add));
    }
};
}  // namespace CDL::Primitive