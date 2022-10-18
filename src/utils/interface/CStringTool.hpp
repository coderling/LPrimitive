#pragma once
#include <string>
#include "CommonDefines.hpp"
#include "DebugUtility.hpp"

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

inline std::wstring Str2Wstr(const char* str, const std::size_t& len)
{
    auto slen = len;
    if (slen == 0)
        {
            slen = std::strlen(str);
        }
    else
        {
            slen = std::strlen(str);
            L_DEV_CHECK_EXPR(len <= slen);
            slen = slen < len ? slen : len;
        }
    std::wostringstream wstm;
    const std::ctype<wchar_t>& ctfacet = use_facet<std::ctype<wchar_t>>(wstm.getloc());
    for (size_t i = 0; i < slen; ++i) wstm << ctfacet.widen(str[i]);
    return wstm.str();
}

inline std::wstring Str2Wstr(const std::string_view& str) { return Str2Wstr(str.data(), str.length()); }

inline std::string Wstr2Str(const wchar_t* wstr, const std::size_t& len)
{
    auto slen = len;
    if (slen == 0)
        {
            slen = std::wcslen(wstr);
        }
    else
        {
            slen = std::wcslen(wstr);
            L_DEV_CHECK_EXPR(len <= slen);
            slen = slen < len ? slen : len;
        }

    std::ostringstream stm;

    // Incorrect code from the link
    // const ctype<char>& ctfacet = use_facet<ctype<char>>(stm.getloc());

    // Correct code.
    const std::ctype<wchar_t>& ctfacet = use_facet<std::ctype<wchar_t>>(stm.getloc());

    for (size_t i = 0; i < slen; ++i) stm << ctfacet.narrow(wstr[i], 0);
    return stm.str();
}

inline std::string Wstr2Str(const std::wstring_view& wstr) { return Wstr2Str(wstr.data(), wstr.length()); }

/*
NODISCARD inline char* CopyString(const char* cstr, const std::size_t& len)
{
    auto slen = len;
    if (slen == 0)
        {
            slen = std::strlen(cstr);
        }
    else
        {
            slen = std::strlen(cstr);
            L_DEV_CHECK_EXPR(len <= slen);
            slen = slen < len ? slen : len;
        }

    auto buffer = new char[slen + 1];
    strcpy_s(buffer, slen + 1, cstr);
    buffer[slen] = '\0';

    return buffer;
}

NODISCARD inline char* CopyString(const std::string_view& str) { return CopyString(str.data(), str.length()); }

inline wchar_t* CopyString(const wchar_t* cstr, const std::size_t& len)
{
    auto slen = len;
    if (slen == 0)
        {
            slen = std::wcslen(cstr);
        }
    else
        {
            slen = std::wcslen(cstr);
            L_DEV_CHECK_EXPR(len <= slen);
            slen = slen < len ? slen : len;
        }

    auto buffer = new wchar_t[slen + 1];
    wcscpy_s(buffer, slen, cstr);
    buffer[slen] = '\0';

    return buffer;
}

inline wchar_t* CopyString(const std::wstring_view& str) { return CopyString(str.data(), str.length()); }
*/
}  // namespace CDL::Primitive