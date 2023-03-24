#pragma once
#include <string.h>

namespace CDL::Primitive
{
inline bool Num(const char& c) { return c >= '0' && c <= '9'; }

inline bool CStrEqual(const char* lstr, const char* rstr)
{
    if ((lstr == nullptr) != (rstr == nullptr))
        {
            return false;
        }

    if (lstr != nullptr && rstr != nullptr)
        {
            return strcmp(lstr, rstr) == 0;
        }

    return true;
}
}  // namespace CDL::Primitive