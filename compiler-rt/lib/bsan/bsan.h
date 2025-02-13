#ifndef BSAN_H
#define BSAN_H

#include "bsan_rt.h"
#include "interception/interception.h"
#include "sanitizer_common/sanitizer_common.h"
#include "sanitizer_common/sanitizer_libc.h"
#include "sanitizer_common/sanitizer_platform_interceptors.h"
#include "sanitizer_common/sanitizer_internal_defs.h"

using __sanitizer::uptr;

namespace __bsan {
    extern bool bsan_inited;
    extern bool bsan_init_is_running;
    void InitializeInterceptors();
} // namespace __bsan

#endif