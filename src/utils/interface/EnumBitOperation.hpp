#pragma once

#ifdef __cplusplus
#include <concepts>

#define DEFINE_BIT_OPEATOR_AR(f)                                                                                                           \
    template <ENUM_BITABLE EType>                                                                                                          \
    inline EType operator f(const EType& lhs, const EType& rhs)                                                                            \
    {                                                                                                                                      \
        using T = typename std::underlying_type<EType>::type;                                                                              \
        return static_cast<EType>(static_cast<T>(lhs) f static_cast<T>(rhs));                                                              \
    }

#define DEFINE_BIT_OPEATOR_AS(l, f)                                                                                                        \
    template <ENUM_BITABLE EType>                                                                                                          \
    inline EType& operator f(const EType& lhs, const EType& rhs)                                                                           \
    {                                                                                                                                      \
        return lhs l rhs;                                                                                                                  \
    }

#define CN(f, e) f##e

#define DEFINE_BIT_OPEATOR(f)                                                                                                              \
    DEFINE_BIT_OPEATOR_AR(f)                                                                                                               \
    DEFINE_BIT_OPEATOR_AS(f, CN(f, =))

template <typename EType>
concept ENUM_BITABLE = std::is_integral_v<std::underlying_type_t<EType>>;

template <ENUM_BITABLE EType>
inline EType operator~(const EType& lhs)
{
    using T = typename std::underlying_type<EType>::type;
    return static_cast<EType>(~(static_cast<T>(lhs)));
}

DEFINE_BIT_OPEATOR(|)
DEFINE_BIT_OPEATOR(&)
DEFINE_BIT_OPEATOR(^)
DEFINE_BIT_OPEATOR(<<)
DEFINE_BIT_OPEATOR(>>)

#else
#endif