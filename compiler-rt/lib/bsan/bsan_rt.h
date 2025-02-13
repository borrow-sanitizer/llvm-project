#ifndef BSANRT_H
#define BSANRT_H

#include <stdint.h>

#ifdef __cplusplus
namespace bsan_rt {
#endif  // __cplusplus

/**
 * The underlying mutability of an allocation
 */
typedef enum Mutability {
  Const,
  Mut,
} Mutability;

/**
 * The unique identifier for an allocation.
 */
typedef uint64_t AllocId;

/**
 * Links a pointer to its node within the tree.
 */
typedef uint64_t BorTag;

/**
 * The metadata associated with an allocation.
 */
typedef struct AllocInfo {
  AllocId id;
  uint64_t size;
  void *base_address;
  /**
   * The tree remains uninitialized (null) until the allocation is
   * borrowed for the first time. Prior to that point, `mutability` is
   * used to check each access.
   */
  enum Mutability mutability;
  void *tree;
} AllocInfo;

/**
 * Each pointer is associated with provenance,
 * which identifies its permission to access memory.
 */
typedef struct Provenance {
  AllocId id;
  BorTag tag;
  /**
   * Pointer to the allocation's metadata.
   * Provenance is the "key," and AllocInfo is the "lock".
   */
  struct AllocInfo *info;
} Provenance;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void bsan_init(void);

void bsan_expose_tag(void *ptr, const struct Provenance *provenance);

uint64_t bsan_retag(void *ptr, uint8_t retag_kind, uint8_t place_kind);

void bsan_read(const void *ptr, uint64_t access_size, const struct Provenance *provenance);

void bsan_write(const void *ptr, uint64_t access_size, const struct Provenance *provenance);

/**
 * Removes all protectors for the given function.
 */
void bsan_func_entry(void);

/**
 * Removes all protectors for the given function.
 */
void bsan_func_exit(void);

/**
 * Performs a deallocation access using the given `ptr` and `provenance`.
 * If successful, it deallocates the tree within `alloc_info`.
 */
void bsan_dealloc(void *ptr, const struct Provenance *provenance);

/**
 * Allocates metadata for an allocation spanning `size` bytes
 * starting at the address `ptr`.
 */
void *bsan_alloc(void *ptr, uint64_t size);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#ifdef __cplusplus
}  // namespace bsan_rt
#endif  // __cplusplus

#endif  /* BSANRT_H */
