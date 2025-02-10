#ifndef BSANRT_H
#define BSANRT_H

#include <stdint.h>

namespace bsanrt {

enum class Mutability {
  Const,
  Mut,
};

using AllocId = uint64_t;

/// Tracking pointer provenance
using BorTag = uint64_t;

struct AllocInfo {
  AllocId id;
  uint64_t size;
  void *base_address;
  Mutability mutability;
  void *tree;
};

struct Provenance {
  AllocId id;
  BorTag tag;
  AllocInfo *info;
};

struct Span {
  uint32_t file;
  uint32_t line;
  const int8_t *file_name;
};

using Malloc = void*(*)(uintptr_t size);

using Free = void(*)(void *ptr);

struct BsanAlloc {
  Malloc alloc;
  Free dealloc;
};

extern "C" {

void bsan_init();

void bsan_expose_tag(void *ptr, const Provenance *provenance, AllocInfo *alloc_info);

uint64_t bsan_retag(void *ptr,
                    uint8_t retag_kind,
                    uint8_t place_kind,
                    BsanAlloc alloc);

void bsan_read(const void *ptr,
               uint64_t access_size,
               const Provenance *provenance,
               AllocInfo *alloc_info);

void bsan_write(const void *ptr,
                uint64_t access_size,
                const Provenance *provenance,
                AllocInfo *alloc_info);

void bsan_func_entry();

void bsan_func_exit();

void bsan_dealloc(void *ptr, const Provenance *provenance, AllocInfo *alloc_info);

void *bsan_alloc(uint64_t size);

}  // extern "C"

}  // namespace bsanrt

#endif  // BSANRT_H
