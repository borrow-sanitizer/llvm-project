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
    extern const bsan_rt::BsanAllocator gBsanAlloc;
    extern bool bsan_initialized;
    extern bool bsan_init_is_running;
    void InitializeInterceptors();
} // namespace __bsan

extern "C" SANITIZER_INTERFACE_ATTRIBUTE void __bsan_preinit();

#endif