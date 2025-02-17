#ifndef BSAN_INTERCEPTORS_H
#define BSAN_INTERCEPTORS_H

#include "bsan.h"
#include "bsan_rt.h"
#include "interception/interception.h"
#include "sanitizer_common/sanitizer_platform.h"
#include "sanitizer_common/sanitizer_platform_interceptors.h"

#  if !SANITIZER_APPLE
#    define BSAN_INTERCEPT_FUNC(name)                                        \
      do {                                                                   \
        if (!INTERCEPT_FUNCTION(name))                                       \
          VReport(1, "BorrowSanitizer: failed to intercept '%s'\n", #name); \
      } while (0)
#    define BSAN_INTERCEPT_FUNC_VER(name, ver)                           \
      do {                                                               \
        if (!INTERCEPT_FUNCTION_VER(name, ver))                          \
          VReport(1, "BorrowSanitizer: failed to intercept '%s@@%s'\n", \
                  #name, ver);                                           \
      } while (0)
#    define BSAN_INTERCEPT_FUNC_VER_UNVERSIONED_FALLBACK(name, ver)           \
      do {                                                                    \
        if (!INTERCEPT_FUNCTION_VER(name, ver) && !INTERCEPT_FUNCTION(name))  \
          VReport(1,                                                          \
                  "BorrowSanitizer: failed to intercept '%s@@%s' or '%s'\n", \
                  #name, ver, #name);                                         \
      } while (0)
#  else
// OS X interceptors don't need to be initialized with INTERCEPT_FUNCTION.
#    define BSAN_INTERCEPT_FUNC(name)
#  endif  // SANITIZER_APPLE

DECLARE_REAL_AND_INTERCEPTOR(void *, malloc, uptr)
DECLARE_REAL_AND_INTERCEPTOR(void, free, void *)
#endif