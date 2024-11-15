#include "bsan_interface_internal.h"
#include "sanitizer_common/sanitizer_common.h"

using namespace __sanitizer;

void __bsan_init() {}

void __bsan_report() {}

void __bsan_abort() {
  Die();
}