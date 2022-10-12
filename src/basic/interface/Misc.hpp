#pragma once
#include <stdint.h>

namespace Toy
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
};
}  // namespace Toy