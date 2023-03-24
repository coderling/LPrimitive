#pragma once
#include <EASTL/string.h>
#include <iostream>
#include "FormatString.hpp"

namespace CDL::Primitive
{
enum ELOG_LEVEL
{
    LOG_LV_INFO = 0,
    LOG_LV_WARNING,
    LOG_LV_ERROR,
    LOG_LV_FATAL_ERROR
};

template <bool>
inline void ThrowIf(const eastl::string &&msg)
{
}

template <>
inline void ThrowIf<true>(const eastl::string &&msg)
{
    throw std::runtime_error(msg.c_str());
}

template <bool throwException, typename... ArgsType>
void Log(ELOG_LEVEL level, const char *file, const char *function, const int &line, const ArgsType... args)
{
    std::stringstream sst;
    switch (level)
        {
            case LOG_LV_WARNING:
                sst << "[Warning]>";
                break;
            case LOG_LV_ERROR:
                sst << "[Error]>";
                break;
            case LOG_LV_FATAL_ERROR:
                sst << "[Fatal Error]>";
                break;
            default:
                sst << "[Info]>";
                break;
        }

    sst << "in " << file << ":" << line << " Func:" << function << "  :";

    const auto &msg = CDL::Primitive::ConcatString(args...);
    if (level >= LOG_LV_ERROR)
        {
            std::cout << sst.str() << msg.c_str() << std::endl;
        }
    else
        {
            std::cout << sst.str() << msg.c_str() << std::endl;
        }

    ThrowIf<throwException>(std::move(msg));
}

template <bool throwException, typename... ArgsType>
void LogFormat(
    ELOG_LEVEL level, const char *file, const char *function, const int &line, const eastl::string_view &format, const ArgsType &...args)
{
    const auto &msg = CDL::Primitive::FormatString(format, args...);
    Log<throwException>(level, file, function, line, msg.c_str());
}

#if PLATFORM_WINDOWS
#define __T_FUNC__ ""
#else
#define __T_FUNC__ __FUNCTION__
#endif

#define LOG(level, exception, ...)                                                                                                         \
    do                                                                                                                                     \
        {                                                                                                                                  \
            CDL::Primitive::Log<exception>(level, __FILE__, __T_FUNC__, __LINE__ __VA_OPT__(, )##__VA_ARGS__);                             \
    } while (false)

#define LOG_FORMAT(level, exception, format, ...)                                                                                          \
    do                                                                                                                                     \
        {                                                                                                                                  \
            CDL::Primitive::LogFormat<exception>(level, __FILE__, __T_FUNC__, __LINE__ __VA_OPT__(, ) format, ##__VA_ARGS__);              \
    } while (false)

#define LOG_INFO(...)                                                                                                                      \
    do                                                                                                                                     \
        {                                                                                                                                  \
            LOG(CDL::Primitive::LOG_LV_INFO, false __VA_OPT__(, )##__VA_ARGS__);                                                           \
    } while (false)

#define LOG_INFO_EXPR(expr, ...)                                                                                                           \
    do                                                                                                                                     \
        {                                                                                                                                  \
            if (!(expr))                                                                                                                   \
                {                                                                                                                          \
                    LOG_INFO(##__VA_ARGS__);                                                                                               \
                }                                                                                                                          \
    } while (false)

#define LOG_INFO_FORMAT(format, ...)                                                                                                       \
    do                                                                                                                                     \
        {                                                                                                                                  \
            LOG_FORMAT(CDL::Primitive::LOG_LV_INFO, false, format __VA_OPT__(, )##__VA_ARGS__);                                            \
    } while (false)

#define LOG_INFO_EXPR_FORMAT(expr, format, ...)                                                                                            \
    do                                                                                                                                     \
        {                                                                                                                                  \
            if (!(expr))                                                                                                                   \
                {                                                                                                                          \
                    LOG_INFO(format __VA_OPT__(, )##__VA_ARGS__);                                                                          \
                }                                                                                                                          \
    } while (false)

#define LOG_WARNING(...)                                                                                                                   \
    do                                                                                                                                     \
        {                                                                                                                                  \
            LOG(CDL::Primitive::LOG_LV_WARNING, false __VA_OPT__(, )##__VA_ARGS__);                                                        \
    } while (false)

#define LOG_WARNING_EXPR(expr, ...)                                                                                                        \
    do                                                                                                                                     \
        {                                                                                                                                  \
            if (!(expr))                                                                                                                   \
                {                                                                                                                          \
                    LOG_WARNING(expr __VA_OPT__(, )##__VA_ARGS__);                                                                         \
                }                                                                                                                          \
    } while (false)

#define LOG_WARNING_FORMAT(format, ...)                                                                                                    \
    do                                                                                                                                     \
        {                                                                                                                                  \
            LOG_FORMAT(CDL::Primitive::LOG_LV_WARNING, false, format __VA_OPT__(, )##__VA_ARGS__);                                         \
    } while (false)

#define LOG_WARNING_EXPR_FORMAT(expr, format, ...)                                                                                         \
    do                                                                                                                                     \
        {                                                                                                                                  \
            if (!(expr))                                                                                                                   \
                {                                                                                                                          \
                    LOG_WARNING_FORMAT(format __VA_OPT__(, )##__VA_ARGS__);                                                                \
                }                                                                                                                          \
    } while (false)

#define LOG_ERROR(...)                                                                                                                     \
    do                                                                                                                                     \
        {                                                                                                                                  \
            LOG(CDL::Primitive::LOG_LV_ERROR, false __VA_OPT__(, )##__VA_ARGS__);                                                          \
    } while (false)

#define LOG_ERROR_EXPR(expr, ...)                                                                                                          \
    do                                                                                                                                     \
        {                                                                                                                                  \
            if (!(expr))                                                                                                                   \
                {                                                                                                                          \
                    LOG_ERROR(##__VA_ARGS__);                                                                                              \
                }                                                                                                                          \
    } while (false)

#define LOG_ERROR_EXCEPTION(...)                                                                                                           \
    do                                                                                                                                     \
        {                                                                                                                                  \
            LOG(CDL::Primitive::LOG_LV_ERROR, true __VA_OPT__(, )##__VA_ARGS__);                                                           \
    } while (false)

#define LOG_ERROR_EXPR_EXCEPTION(expr, format, ...)                                                                                        \
    do                                                                                                                                     \
        {                                                                                                                                  \
            if (!(expr))                                                                                                                   \
                {                                                                                                                          \
                    LOG_ERROR_EXCEPTION(format __VA_OPT__(, )##__VA_ARGS__);                                                               \
                }                                                                                                                          \
    } while (false)

#define LOG_ERROR_FORMAT(format, ...)                                                                                                      \
    do                                                                                                                                     \
        {                                                                                                                                  \
            LOG_FORMAT(CDL::Primitive::LOG_LV_ERROR, false, format __VA_OPT__(, )##__VA_ARGS__);                                           \
    } while (false)

#define LOG_ERROR_EXPR_FORMAT(expr, format, ...)                                                                                           \
    do                                                                                                                                     \
        {                                                                                                                                  \
            if (!(expr))                                                                                                                   \
                {                                                                                                                          \
                    LOG_ERROR_FORMAT(format __VA_OPT__(, )##__VA_ARGS__);                                                                  \
                }                                                                                                                          \
    } while (false)

#define LOG_ERROR_EXCEPTION_FORMAT(format, ...)                                                                                            \
    do                                                                                                                                     \
        {                                                                                                                                  \
            LOG_FORMAT(CDL::Primitive::LOG_LV_ERROR, true, format __VA_OPT__(, )##__VA_ARGS__);                                            \
    } while (false)

#define LOG_ERROR_EXPR_EXCEPTION_FORMAT(expr, format, ...)                                                                                 \
    do                                                                                                                                     \
        {                                                                                                                                  \
            if (!(expr))                                                                                                                   \
                {                                                                                                                          \
                    LOG_ERROR_EXCEPTION_FORMAT(format __VA_OPT__(, )##__VA_ARGS__);                                                        \
                }                                                                                                                          \
    } while (false)

#define LOG_FATAL_ERROR(...)                                                                                                               \
    do                                                                                                                                     \
        {                                                                                                                                  \
            LOG(CDL::Primitive::LOG_LV_FATAL_ERROR, false __VA_OPT__(, )##__VA_ARGS__);                                                    \
    } while (false)

#define LOG_FATAL_ERROR_EXPR(expr, format, ...)                                                                                            \
    do                                                                                                                                     \
        {                                                                                                                                  \
            if (!(expr))                                                                                                                   \
                {                                                                                                                          \
                    LOG_FATAL_ERROR(format __VA_OPT__(, )##__VA_ARGS__);                                                                   \
                }                                                                                                                          \
    } while (false)

#define LOG_FATAL_ERROR_EXCEPTION(...)                                                                                                     \
    do                                                                                                                                     \
        {                                                                                                                                  \
            LOG(CDL::Primitive::LOG_LV_FATAL_ERROR, true __VA_OPT__(, )##__VA_ARGS__);                                                     \
    } while (false)

#define LOG_FATAL_ERROR_EXPR_EXCEPTION(expr, ...)                                                                                          \
    do                                                                                                                                     \
        {                                                                                                                                  \
            if (!(expr))                                                                                                                   \
                {                                                                                                                          \
                    LOG(CDL::Primitive::LOG_LV_FATAL_ERROR, true __VA_OPT__(, )##__VA_ARGS__);                                             \
                }                                                                                                                          \
    } while (false)

#define LOG_FATAL_ERROR_FORMAT(format, ...)                                                                                                \
    do                                                                                                                                     \
        {                                                                                                                                  \
            LOG_FORMAT(CDL::Primitive::LOG_LV_FATAL_ERROR, false, format __VA_OPT__(, )##__VA_ARGS__);                                     \
    } while (false)

#define LOG_FATAL_ERROR_EXPR_FORMAT(expr, format, ...)                                                                                     \
    do                                                                                                                                     \
        {                                                                                                                                  \
            if (!(expr))                                                                                                                   \
                {                                                                                                                          \
                    LOG_FATAL_ERROR_FORMAT(format __VA_OPT__(, )##__VA_ARGS__);                                                            \
                }                                                                                                                          \
    } while (false)

#define LOG_FATAL_ERROR_EXCEPTION_FORMAT(format, ...)                                                                                      \
    do                                                                                                                                     \
        {                                                                                                                                  \
            LOG_FORMAT(CDL::Primitive::LOG_LV_FATAL_ERROR, true, format __VA_OPT__(, )##__VA_ARGS__);                                      \
    } while (false)

#define LOG_FATAL_ERROR_EXPR_EXCEPTION_FORMAT(expr, format, ...)                                                                           \
    do                                                                                                                                     \
        {                                                                                                                                  \
            if (!(expr))                                                                                                                   \
                {                                                                                                                          \
                    LOG_FATAL_ERROR_EXCEPTION_FORMAT(format __VA_OPT__(, )##__VA_ARGS__);                                                  \
                }                                                                                                                          \
    } while (false)
}  // namespace CDL::Primitive