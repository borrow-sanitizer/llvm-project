#ifndef BSANRT_H
#define BSANRT_H

#include <stdint.h>

#ifdef __cplusplus
namespace bsan_rt {
#endif  // __cplusplus

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void bsan_init(void);

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
