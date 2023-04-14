#pragma once

#include <format>
#include <iostream>
#include <sstream>
#include <string>

namespace DT::Primitive
{
#define STRINGIFY(x) #x
#define STRINGIFY_BUILTIN(x) STRINGIFY(x)

template <typename StreamType, typename FArgsType, typename... ArgsType>
void StreamOutput(StreamType& st, const FArgsType f_args, const ArgsType&... args)
{
    return (st << ... << args);
}

template <typename StreamType, typename... ArgsType>
void StreamFormatOutput(StreamType& st, const std::string_view& format, const ArgsType&... args)
{
    auto o = std::vformat(format.data(), std::make_format_args(args...));
    StreamOutput(st, o);
}

template <typename... ArgsType>
void FormatOutput(const std::string_view& format, const ArgsType&... args)
{
    StreamFormatOutput(std::cout, format, args...);
}

template <typename... ArgsType>
std::string FormatString(const std::string_view& format, const ArgsType&... args)
{
    std::stringstream sst;
    StreamFormatOutput(sst, format, args...);
    return std::string(sst.str().data());
}

template <typename... ArgsType>
std::string ConcatString(const ArgsType&... args)
{
    std::stringstream sst;
    return (sst << ... << args);
}
}  // namespace DT::Primitive