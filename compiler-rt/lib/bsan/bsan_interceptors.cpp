
#include "bsan_interceptors.h"
#include "bsan.h"
#include "bsanrt.h"
#include "sanitizer_common/sanitizer_common.h"

using namespace __sanitizer;
static bool interceptors_initialized = false;

using namespace __bsan;

INTERCEPTOR(void *, malloc, SIZE_T size) {
  bsanrt::bsan_alloc((intptr_t)size);
  return REAL(malloc)(size);
}

INTERCEPTOR(void, free, void *ptr) {
  bsanrt::bsan_dealloc(ptr, 0, 0);
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
