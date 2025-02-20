#include "bsan.h"
#include "bsan_rt.h"
#include "sanitizer_common/sanitizer_internal_defs.h"

using namespace __bsan;

#if SANITIZER_CAN_USE_PREINIT_ARRAY
__attribute__((section(".preinit_array"), used)) static auto bsan_preinit =
    __bsan_preinit;
#endif
