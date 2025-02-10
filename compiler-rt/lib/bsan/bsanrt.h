#ifndef BSANRT_H
#define BSANRT_H

#include <stdint.h>

namespace bsanrt {

enum class Mutability {
  Const,
  Mut,
};

using Malloc = void*(*)(uintptr_t size);

using Free = void(*)(void *ptr);

struct BsanAlloc {
  Malloc alloc;
  Free dealloc;
};

using AllocId = uintptr_t;

/// Tracking pointer provenance
using BorTag = uintptr_t;

struct AllocInfo {
  AllocId id;
  uintptr_t size;
  void *base_address;
  Mutability mutability;
  void *tree;
};

struct Provenance {
  AllocId id;
  BorTag tag;
  AllocInfo *info;
};

extern "C" {

void bsan_init();

void bsan_expose_tag(void *ptr, const Provenance *provenance, AllocInfo *alloc_info);

uint64_t bsan_retag(void *ptr, uint8_t retag_kind, uint8_t place_kind, BsanAlloc alloc);

void bsan_read(const void *ptr,
               uintptr_t access_size,
               const Provenance *provenance,
               AllocInfo *alloc_info);

void bsan_write(const void *ptr,
                uintptr_t access_size,
                const Provenance *provenance,
                AllocInfo *alloc_info);

void bsan_func_entry();

void bsan_func_exit();

void bsan_dealloc(void *ptr, const Provenance *provenance, AllocInfo *alloc_info);

void *bsan_alloc(uintptr_t size);

}  // extern "C"

}  // namespace bsanrt

#endif  // BSANRT_H
