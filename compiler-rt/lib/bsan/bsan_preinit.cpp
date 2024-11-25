#include "bsan.h"
#include "sanitizer_common/sanitizer_internal_defs.h"

#if SANITIZER_CAN_USE_PREINIT_ARRAY
__attribute__((section(".preinit_array"), used)) static auto bsan_preinit =
    __bsan_init;
#endif
