#ifndef BSAN_H
#define BSAN_H

#include "borrowtracker.h"
#include "sanitizer_common/sanitizer_common.h"
#include "sanitizer_common/sanitizer_flag_parser.h"
#include "sanitizer_common/sanitizer_flags.h"

#include "interception/interception.h"
#include "sanitizer_common/sanitizer_libc.h"
#include "sanitizer_common/sanitizer_linux.h"
#include "sanitizer_common/sanitizer_platform_interceptors.h"

#include "sanitizer_common/sanitizer_internal_defs.h"

using __sanitizer::uptr;

namespace __bsan {} // namespace __bsan

extern "C" {
SANITIZER_INTERFACE_ATTRIBUTE void __bsan_init();
}

#endif