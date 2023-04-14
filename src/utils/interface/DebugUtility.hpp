#pragma once

#include <cassert>
#include <concepts>
#include <iostream>
#include "FormatString.hpp"
#include "Logger.hpp"

namespace DT::Primitive::Debug
{
#define L_CHECK_OUTPUT(msg, ...)                                                                                                           \
    do                                                                                                                                     \
        {                                                                                                                                  \
            LOG_ERROR(msg __VA_OPT__(, )##__VA_ARGS__);                                                                                    \
    } while (false)

#define L_CHECK_OUTPUT_FORMAT(fmt, ...)                                                                                                    \
    do                                                                                                                                     \
        {                                                                                                                                  \
            LOG_ERROR_FORMAT(fmt __VA_OPT__(, )##__VA_ARGS__);                                                                             \
    } while (false)

#ifdef _DEBUG
#define L_ASSERT_FAILED(msg, ...)                                                                                                          \
    do                                                                                                                                     \
        {                                                                                                                                  \
            LOG_ERROR_EXCEPTION("[Assert Fail]>", msg __VA_OPT__(, )##__VA_ARGS__);                                                        \
    } while (false)

#define L_ASSERT(expr, msg, ...)                                                                                                           \
    do                                                                                                                                     \
        {                                                                                                                                  \
            if (!(expr))                                                                                                                   \
                {                                                                                                                          \
                    L_ASSERT_FAILED(msg __VA_OPT__(, )##__VA_ARGS__);                                                                      \
                }                                                                                                                          \
    } while (false)

#define L_ASSERT_EXPR(expr, ...)                                                                                                           \
    do                                                                                                                                     \
        {                                                                                                                                  \
            L_ASSERT(expr, "check expr:", #expr, " failed " __VA_OPT__(, )##__VA_ARGS__);                                                  \
    } while (false)

#define L_ASSERT_FAILED_FORMAT(fmt, ...)                                                                                                   \
    do                                                                                                                                     \
        {                                                                                                                                  \
            const auto& r_fmt = DT::Primitive::FormatString(fmt __VA_OPT__(, )##__VA_ARGS__);                                              \
            LOG_ERROR("[Assert Fail]>", r_fmt);                                                                                            \
    } while (false)

#define L_ASSERT_FORMAT(expr, fmt, ...)                                                                                                    \
    do                                                                                                                                     \
        {                                                                                                                                  \
            if (!(expr))                                                                                                                   \
                {                                                                                                                          \
                    LOG_ERROR_EXCEPTION_FORMAT(fmt __VA_OPT__(, )##__VA_ARGS__);                                                           \
                }                                                                                                                          \
    } while (false)

#define L_ASSERT_EXPR_FORMAT(expr, fmt, ...)                                                                                               \
    do                                                                                                                                     \
        {                                                                                                                                  \
            const auto& r_fmt = DT::Primitive::FormatString("{}{}{}{}", "check_expr:", #expr, " failed ", fmt);                            \
            L_ASSERT_FORMAT(expr, r_fmt __VA_OPT__(, )##__VA_ARGS__);                                                                      \
    } while (false)

#else
#define L_ASSERT_FAILED(msg, ...)                                                                                                          \
    do                                                                                                                                     \
        {                                                                                                                                  \
    } while (false)
#define L_ASSERT_EXPR(expr, ...)                                                                                                           \
    do                                                                                                                                     \
        {                                                                                                                                  \
    } while (false)
#define L_ASSERT(expr, msg, ...)                                                                                                           \
    do                                                                                                                                     \
        {                                                                                                                                  \
    } while (false)
#define L_ASSERT_FAILED_FORMAT(msg, ...)                                                                                                   \
    do                                                                                                                                     \
        {                                                                                                                                  \
    } while (false)
#define L_ASSERT_EXPR_FORMAT(expr, msg, ...)                                                                                               \
    do                                                                                                                                     \
        {                                                                                                                                  \
    } while (false)
#define L_ASSERT_FORMAT(expr)                                                                                                              \
    do                                                                                                                                     \
        {                                                                                                                                  \
    } while (false)
#endif  // _DEBUG

#ifdef _DEBUG
#define L_DEV_CHECK_EXPR L_ASSERT_EXPR
#define L_DEV_CHECK_EXPR_FORMAT L_ASSERT_EXPR_FORMAT
#elif L_DEVELOPMENT
#define L_DEV_CHECK_EXPR_(expr, ...)                                                                                                       \
    do                                                                                                                                     \
        {                                                                                                                                  \
            if (!(expr))                                                                                                                   \
                {                                                                                                                          \
                    L_CHECK_OUTPUT("check expr:", #expr, " failed" __VA_OPT__(, )##__VA_ARGS__);                                           \
                }                                                                                                                          \
    } while (false)

#define L_DEV_CHECK_EXPR_FORMAT(expr, fmt, ...)                                                                                            \
    do                                                                                                                                     \
        {                                                                                                                                  \
            if (!(expr))                                                                                                                   \
                {                                                                                                                          \
                    const auto& r_fmt = DT::Primitive::FormatString("{}{}{}{}", "check_expr:", #expr, " failed ", fmt);                    \
                    L_CHECK_OUTPUT_FORMAT(r_fmt __VA_OPT__(, )##__VA_ARGS__);                                                              \
                }                                                                                                                          \
    } while (false)
#else
#define L_DEV_CHECK_EXPR(expr, ...)                                                                                                        \
    do                                                                                                                                     \
        {                                                                                                                                  \
    } while (false)
#define L_DEV_CHECK_EXPR_FORMA(expr, fmt, ...)                                                                                             \
    do                                                                                                                                     \
        {                                                                                                                                  \
    } while (false)
#define L_DEV_CHECK_EXPR_FORMAT(expr, fmt, ...)                                                                                            \
    do                                                                                                                                     \
        {                                                                                                                                  \
    } while (false)
#endif

#define L_ASSERT_TR(tr, ...) L_ASSERT_EXPR(tr == TR_OK __VA_OPT__(, )##__VA_ARGS__)

template <typename DestType, typename SourceType>
inline DestType* StaticPointerCast(SourceType* s_ptr)
{
#ifdef DEVELOPMENT
    L_ASSERT(s_ptr == nullptr || dynamic_cast<DestType*>(s_ptr) != nullptr,
             "Dynamic type cast failed. src type: ", typeid(SourceType).name(), typeid(DestType).name());
#endif

    return static_cast<DestType*>(s_ptr);
}

}  // namespace DT::Primitive::Debug