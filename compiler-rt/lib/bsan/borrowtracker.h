#ifndef BORROW_TRACKER_H
#define BORROW_TRACKER_H

#include <stdint.h>

struct TrackedPointer {
  void *addr;
  uint64_t alloc_id;
  uint64_t tag;
};

extern "C" {

void bsan_init();

uint64_t bsan_expose_tag(void *ptr);

uint64_t bsan_retag(void *ptr, uint8_t retag_kind, uint8_t place_kind);

uint64_t bsan_read(void *ptr, uint64_t access_size);

uint64_t bsan_write(void *ptr, uint64_t access_size);

void bsan_func_entry();

void bsan_func_exit();

}  // extern "C"

#endif
