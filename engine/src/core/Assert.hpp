#pragma once

#include "Log.hpp"

// ENGINE_ASSERT — for programmer errors only (precondition violations, etc.).
// Calls LOG_FATAL which prints diagnostics then std::abort().
// Never use for recoverable errors; use std::optional / Result<T,E> instead.
#define ENGINE_ASSERT(cond, msg)                                        \
    do {                                                                \
        if (!(cond)) {                                                  \
            LOG_FATAL("Assert failed: {} | {}", #cond, msg);           \
        }                                                               \
    } while (0)
