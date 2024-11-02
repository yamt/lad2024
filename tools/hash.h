#include <stddef.h>
#include <stdint.h>

uint32_t sdbm_hash(const void *p, size_t len);
uint32_t fletcher32(const void *p, size_t len);
