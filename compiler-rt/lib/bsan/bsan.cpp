#include "bsan.h"
#include "sanitizer_common/sanitizer_common.h"

using namespace __sanitizer;
using namespace __bsan;

extern "C" SANITIZER_INTERFACE_ATTRIBUTE void __bsan_init() {
  return bsan_init();
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE void __bsan_func_entry() {
  return bsan_func_entry();
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE void __bsan_func_exit() {
  return bsan_func_exit();
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE uint64_t
__bsan_retag(void *ptr, uint8_t retag_kind, uint8_t place_kind) {
  return bsan_retag(ptr, retag_kind, place_kind);
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE void
__bsan_write(void *ptr, uint64_t access_size) {
  bsan_write(ptr, access_size);
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE void
__bsan_read(void *ptr, uint64_t access_size) {
  bsan_read(ptr, access_size);
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE void __bsan_expose_tag(void *ptr) {
  bsan_expose_tag(ptr);
}