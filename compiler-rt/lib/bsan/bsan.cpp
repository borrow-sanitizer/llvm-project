#include "bsan.h"
#include "bsan_interceptors.h"
#include "bsan_rt.h"
#include "sanitizer_common/sanitizer_common.h"
#include "sanitizer_common/sanitizer_stacktrace.h"

using namespace __sanitizer;
using namespace __bsan;

namespace __bsan {
bool bsan_inited = false;
bool bsan_init_is_running = false;
bool bsan_deinit_is_running = false;
} // namespace __bsan

extern "C" SANITIZER_INTERFACE_ATTRIBUTE void __bsan_init() {
  CHECK(!bsan_init_is_running);
  if (bsan_inited)
    return;
  bsan_init_is_running = true;
  InitializeInterceptors();
  bsan_rt::bsan_init();
  bsan_inited = true;
  bsan_init_is_running = false;
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE void __bsan_func_entry() {}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE void __bsan_func_exit() {}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE usize __bsan_retag(void *ptr,
                                                            u8 retag_kind,
                                                            u8 place_kind) {
  return bsan_rt::bsan_retag(ptr, retag_kind, place_kind);
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE void __bsan_write(void *ptr,
                                                           u64 access_size) {
  bsan_rt::bsan_write(ptr, access_size, 0);
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE void __bsan_read(void *ptr,
                                                          u64 access_size) {
  bsan_rt::bsan_read(ptr, access_size, 0);
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE void __bsan_expose_tag(void *ptr) {
  bsan_rt::bsan_expose_tag(ptr, 0);
}