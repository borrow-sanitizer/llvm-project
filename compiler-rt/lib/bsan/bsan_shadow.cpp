#include "bsan.h"

using namespace __sanitizer;

namespace __bsan {

static atomic_uintptr_t next_alloc_id;

void InitializeShadowMemory() {
  atomic_store(&next_alloc_id, 1, memory_order_relaxed);
}

uptr NextAllocId() {
  return atomic_fetch_add(&next_alloc_id, 1, memory_order_relaxed);
}

void DestroyShadowMemory() {}

} // namespace __bsan