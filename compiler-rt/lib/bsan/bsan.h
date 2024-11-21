#ifndef BSAN_H
#define BSAN_H

#include "borrowtracker.h"
#include "sanitizer_common/sanitizer_common.h"
#include "sanitizer_common/sanitizer_flag_parser.h"
#include "sanitizer_common/sanitizer_flags.h"

#include "interception/interception.h"
#include "sanitizer_common/sanitizer_platform_interceptors.h"
#include "sanitizer_common/sanitizer_libc.h"
#include "sanitizer_common/sanitizer_linux.h" 

#include "sanitizer_common/sanitizer_internal_defs.h"

using __sanitizer::uptr;


namespace __bsan {


}  // namespace __bsan





#endif

#if SANITIZER_CAN_USE_PREINIT_ARRAY
// This section is linked into the main executable when -fsanitize=borrow is
// specified to perform initialization at a very early stage.
__attribute__((section(".preinit_array"), used)) static auto preinit =
    __bsan_init;
#endif