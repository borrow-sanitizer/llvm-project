#include "bsan.h"
#include "bsan_rt.h"
#include "bsan_interceptors.h"
#include "interception/interception.h"
#include "sanitizer_common/sanitizer_common.h"
#include "sanitizer_common/sanitizer_allocator_dlsym.h"
#include "sanitizer_common/sanitizer_flags.h"
#include "sanitizer_common/sanitizer_internal_defs.h"
#include "sanitizer_common/sanitizer_linux.h"
#include "sanitizer_common/sanitizer_platform_interceptors.h"

#include <stddef.h>

using namespace __sanitizer;
static bool interceptors_initialized = false;
using namespace __bsan;

namespace {
  struct DlsymAlloc : public DlSymAllocator<DlsymAlloc> {
    static bool UseImpl() { return !bsan_initialized; }
  };
  } // namespace

INTERCEPTOR(void *, malloc, SIZE_T size) {
  if (DlsymAlloc::Use())
    return DlsymAlloc::Allocate(size);
  return REAL(malloc)(size);
}

INTERCEPTOR(void, free, void *ptr) {
  if (DlsymAlloc::PointerIsMine(ptr))
    return DlsymAlloc::Free(ptr);
  return REAL(free)(ptr);
}

namespace __bsan {
void InitializeInterceptors() {
  CHECK(!interceptors_initialized);
  __interception::DoesNotSupportStaticLinking();

  BSAN_INTERCEPT_FUNC(malloc);
  BSAN_INTERCEPT_FUNC(free);
  interceptors_initialized = true;
}
} // namespace __bsan
