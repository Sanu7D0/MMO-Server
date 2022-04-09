#pragma once
#include "Types.h"

#define CRASH(cause) \
    {                \
        abort();     \
    }

#define ASSERT_CRASH(expr)        \
    {                             \
        if (!(expr)) {            \
            CRASH("ASSERT_CRASH") \
        }                         \
    }