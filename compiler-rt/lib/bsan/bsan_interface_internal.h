#ifndef BSAN_INTERFACE_INTERNAL_H
#define BSAN_INTERFACE_INTERNAL_H

#include "sanitizer_common/sanitizer_internal_defs.h"

using __sanitizer::uptr;

extern "C" {
SANITIZER_INTERFACE_ATTRIBUTE
void __bsan_init();

SANITIZER_INTERFACE_ATTRIBUTE
void __bsan_report();

SANITIZER_INTERFACE_ATTRIBUTE __attribute__((noreturn))
void __bsan_abort();

}

#endif // ODEF_INTERFACE_INTERNAL_H