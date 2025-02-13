
#include "bsan.h"
#include "bsan_rt.h"
#include "bsan_interceptors.h"
#include "sanitizer_common/sanitizer_common.h"

using namespace __sanitizer;
static bool interceptors_initialized = false;

using namespace __bsan;

INTERCEPTOR(void *, malloc, SIZE_T size) {
  void * ptr = REAL(malloc)(size);
  bsan_rt::bsan_alloc(ptr, size);
  return ptr;
}

INTERCEPTOR(void, free, void *ptr) {
  bsan_rt::bsan_dealloc(ptr, 0);
  return REAL(free)(ptr);
}

namespace __bsan {
void InitializeInterceptors() {
  CHECK(!interceptors_initialized);
  BSAN_INTERCEPT_FUNC(malloc);
  BSAN_INTERCEPT_FUNC(free);
  interceptors_initialized = true;
}
} // namespace __bsan
