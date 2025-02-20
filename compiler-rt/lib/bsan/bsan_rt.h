#ifndef BSANRT_H
#define BSANRT_H

#include <stdint.h>

#ifdef __cplusplus
namespace bsan_rt {
#endif  // __cplusplus

typedef void *(*Malloc)(uintptr_t);

typedef void (*Free)(void*);

typedef void *(*MMap)(void*, int, int, int, unsigned long long);

typedef int (*MUnmap)(void*, uintptr_t);

typedef struct BsanAllocator {
  Malloc malloc;
  Free free;
  MMap mmap;
  MUnmap munmap;
} BsanAllocator;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void bsan_init(struct BsanAllocator alloc);

void bsan_expose_tag(void *ptr);

uint64_t bsan_retag(void *ptr, uint8_t retag_kind, uint8_t place_kind);

void bsan_read(void *ptr, uint64_t access_size);

void bsan_write(void *ptr, uint64_t access_size);

void bsan_func_entry(void);

void bsan_func_exit(void);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#ifdef __cplusplus
}  // namespace bsan_rt
#endif  // __cplusplus

#endif  /* BSANRT_H */
