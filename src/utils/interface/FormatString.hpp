#pragma once

#include <EASTL/string.h>
#include <EASTL/string_view.h>
#include <format>
#include <iostream>
#include <sstream>

namespace CDL::Primitive
{
#define STRINGIFY(x) #x
#define STRINGIFY_BUILTIN(x) STRINGIFY(x)

template <typename StreamType, typename ArgsType>
inline void StreamOutput(StreamType& st)
{
}

template <typename StreamType, typename ArgsType>
inline void StreamOutput(StreamType& st, const ArgsType& args)
{
    st << args;
}

template <typename StreamType, typename FArgsType, typename... ArgsType>
void StreamOutput(StreamType& st, const FArgsType f_args, const ArgsType&... args)
{
    StreamOutput(st, f_args);
    StreamOutput(st, args...);
}

template <typename StreamType, typename... ArgsType>
void StreamFormatOutput(StreamType& st, const eastl::string_view& format, const ArgsType&... args)
{
    auto o = std::vformat(format.data(), std::make_format_args(args...));
    StreamOutput(st, o);
}

template <typename... ArgsType>
void FormatOutput(const eastl::string_view& format, const ArgsType&... args)
{
    StreamFormatOutput(std::cout, format, args...);
}

template <typename... ArgsType>
eastl::string FormatString(const eastl::string_view& format, const ArgsType&... args)
{
    std::stringstream sst;
    StreamFormatOutput(sst, format, args...);
    return eastl::string(sst.str().data());
}

template <typename... ArgsType>
eastl::string ConcatString(const ArgsType&... args)
{
    std::stringstream sst;
    StreamOutput(sst, args...);
    return eastl::string(sst.str().data());
}
}  // namespace CDL::Primitive