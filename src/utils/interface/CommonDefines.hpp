#pragma once
#include <stdint.h>

#define NODISCARD [[nodiscard]]
#define NODISCARD_MSG(msg) [[nodiscard(msg)]]

#define NOCOPY_INPLACE(CLS_NAME)                                                                                                           \
    CLS_NAME(const CLS_NAME&) = delete;                                                                                                    \
    CLS_NAME& operator=(const CLS_NAME&) = delete

#define NOMOVE_INPLACE(CLS_NAME)                                                                                                           \
    CLS_NAME(CLS_NAME&&) = delete;                                                                                                         \
    CLS_NAME& operator=(CLS_NAME&&) = delete

#define NOCOPY_AND_NOMOVE(CLS_NAME)                                                                                                        \
    NOCOPY_INPLACE(CLS_NAME);                                                                                                              \
    NOMOVE_INPLACE(CLS_NAME)

enum class L_RESULT
{
    TR_ERROR = -1,
    TR_OK = 0
};

namespace CDL::Primitive
{
inline bool Succeed(const L_RESULT& tr) { return tr >= static_cast<L_RESULT>(0); }

inline bool Failed(const L_RESULT& tr) { return !Succeed(tr); }
}  // namespace CDL::Primitive