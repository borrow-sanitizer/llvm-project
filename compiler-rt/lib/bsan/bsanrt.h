#ifndef BSANRT_H
#define BSANRT_H

#include <stdint.h>

/// A thread identifier.
struct ThreadId;



extern "C" {

void bsan_init();

void bsan_expose_tag(void *ptr);

uint64_t bsan_retag(void *ptr, uint8_t retag_kind, uint8_t place_kind);

void bsan_read(void *ptr, uint64_t access_size);

void bsan_write(void *ptr, uint64_t access_size);

void bsan_func_entry();

void bsan_func_exit();

}  // extern "C"

#endif  // BSANRT_H
