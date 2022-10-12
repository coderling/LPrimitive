#pragma once

#include <cassert>
#include <concepts>
#include <iostream>
#include "FormatString.hpp"
#include "Logger.hpp"

namespace Toy::Debug
{
#define ENGINE_CHECK_OUTPUT(msg, ...)                                                                                                      \
    do                                                                                                                                     \
        {                                                                                                                                  \
            LOG_ERROR(msg __VA_OPT__(, )##__VA_ARGS__);                                                                                    \
    } while (false)

#define ENGINE_CHECK_OUTPUT_FORMAT(fmt, ...)                                                                                               \
    do                                                                                                                                     \
        {                                                                                                                                  \
            LOG_ERROR_FORMAT(fmt __VA_OPT__(, )##__VA_ARGS__);                                                                             \
    } while (false)

#ifdef ENGINE_DEBUG
#define ENGINE_ASSERT_FAILED(msg, ...)                                                                                                     \
    do                                                                                                                                     \
        {                                                                                                                                  \
            LOG_ERROR_EXCEPTION("[Assert Fail]>", msg __VA_OPT__(, )##__VA_ARGS__);                                                        \
    } while (false)

#define ENGINE_ASSERT(expr, msg, ...)                                                                                                      \
    do                                                                                                                                     \
        {                                                                                                                                  \
            if (!(expr))                                                                                                                   \
                {                                                                                                                          \
                    ENGINE_ASSERT_FAILED(msg __VA_OPT__(, )##__VA_ARGS__);                                                                 \
                }                                                                                                                          \
    } while (false)

#define ENGINE_ASSERT_EXPR(expr, ...)                                                                                                      \
    do                                                                                                                                     \
        {                                                                                                                                  \
            ENGINE_ASSERT(expr, "check expr:", #expr, " failed " __VA_OPT__(, )##__VA_ARGS__);                                             \
    } while (false)

#define ENGINE_ASSERT_FAILED_FORMAT(fmt, ...)                                                                                              \
    do                                                                                                                                     \
        {                                                                                                                                  \
            const auto& r_fmt = Toy::FormatString(fmt __VA_OPT__(, )##__VA_ARGS__);                                                        \
            LOG_ERROR("[Assert Fail]>", r_fmt);                                                                                            \
    } while (false)

#define ENGINE_ASSERT_FORMAT(expr, fmt, ...)                                                                                               \
    do                                                                                                                                     \
        {                                                                                                                                  \
            if (!(expr))                                                                                                                   \
                {                                                                                                                          \
                    LOG_ERROR_EXCEPTION_FORMAT(fmt __VA_OPT__(, )##__VA_ARGS__);                                                           \
                }                                                                                                                          \
    } while (false)

#define ENGINE_ASSERT_EXPR_FORMAT(expr, fmt, ...)                                                                                          \
    do                                                                                                                                     \
        {                                                                                                                                  \
            const auto& r_fmt = Toy::FormatString("{}{}{}{}", "check_expr:", #expr, " failed ", fmt);                                      \
            ENGINE_ASSERT_FORMAT(expr, r_fmt __VA_OPT__(, )##__VA_ARGS__);                                                                 \
    } while (false)

#else
#define ENGINE_ASSERT_FAILED(msg, ...)                                                                                                     \
    do                                                                                                                                     \
        {                                                                                                                                  \
    } while (false)
#define ENGINE_ASSERT_EXPR(expr, ...)                                                                                                      \
    do                                                                                                                                     \
        {                                                                                                                                  \
    } while (false)
#define ENGINE_ASSERT(expr, msg, ...)                                                                                                      \
    do                                                                                                                                     \
        {                                                                                                                                  \
    } while (false)
#define ENGINE_ASSERT_FAILED_FORMAT(msg, ...)                                                                                              \
    do                                                                                                                                     \
        {                                                                                                                                  \
    } while (false)
#define ENGINE_ASSERT_EXPR_FORMAT(expr, msg, ...)                                                                                          \
    do                                                                                                                                     \
        {                                                                                                                                  \
    } while (false)
#define ENGINE_ASSERT_FORMAT(expr)                                                                                                         \
    do                                                                                                                                     \
        {                                                                                                                                  \
    } while (false)
#endif  // ENGINE_DEBUG

#ifdef ENGINE_DEBUG
#define ENGINE_DEV_CHECK_EXPR ENGINE_ASSERT_EXPR
#define ENGINE_DEV_CHECK_EXPR_FORMAT ENGINE_ASSERT_EXPR_FORMAT
#elif ENGINE_DEVELOPMENT
#define ENGINE_DEV_CHECK_EXPR_(expr, ...)                                                                                                  \
    do                                                                                                                                     \
        {                                                                                                                                  \
            if (!(expr))                                                                                                                   \
                {                                                                                                                          \
                    ENGINE_CHECK_OUTPUT("check expr:", #expr, " failed" __VA_OPT__(, )##__VA_ARGS__);                                      \
                }                                                                                                                          \
    } while (false)

#define ENGINE_DEV_CHECK_EXPR_FORMAT(expr, fmt, ...)                                                                                       \
    do                                                                                                                                     \
        {                                                                                                                                  \
            if (!(expr))                                                                                                                   \
                {                                                                                                                          \
                    const auto& r_fmt = Toy::FormatString("{}{}{}{}", "check_expr:", #expr, " failed ", fmt);                              \
                    ENGINE_CHECK_OUTPUT_FORMAT(r_fmt __VA_OPT__(, )##__VA_ARGS__);                                                         \
                }                                                                                                                          \
    } while (false)
#else
#define ENGINE_DEV_CHECK_EXPR(expr, ...)                                                                                                   \
    do                                                                                                                                     \
        {                                                                                                                                  \
    } while (false)
#define ENGINE_DEV_CHECK_EXPR_FORMA(expr, fmt, ...)                                                                                        \
    do                                                                                                                                     \
        {                                                                                                                                  \
    } while (false)
#define ENGINE_DEV_CHECK_EXPR_FORMAT(expr, fmt, ...)                                                                                       \
    do                                                                                                                                     \
        {                                                                                                                                  \
    } while (false)
#endif

#define ENGINE_ASSERT_TR(tr, ...) ENGINE_ASSERT_EXPR(tr == TR_OK __VA_OPT__(, )##__VA_ARGS__)

template <typename DestType, typename SourceType>
inline DestType* CheckDynamicPointerCast(SourceType* s_ptr)
{
#ifdef ENGINE_DEVELOPMENT
    ENGINE_ASSERT(s_ptr == nullptr || dynamic_cast<DestType*>(s_ptr) != nullptr,
                  "Dynamic type cast failed. src type: ", typeid(SourceType).name(), typeid(DestType).name());
#endif

    return static_cast<DestType*>(s_ptr);
}

#define SUCCEEDED_TR(tr) ((tr) == TR_OK)
#define FAILED_TR(tr) ((tr) != TR_OK)

#define ASSERT_TR(tr, ...) ENGINE_ASSERT_EXPR((tr) == TR_OK __VA_OPT__(, ), ##__VA_ARGS__);

}  // namespace Toy::Debug