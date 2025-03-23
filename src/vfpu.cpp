#include "vfpu.hpp"

i32 vfpu_check(const void *ptr) { return ptr != NULL; }

i32 vfpu_aligned_check(const void *ptr, i32 offset) {
    const u8 *bptr = ((u8 *)ptr) + offset;
    i32 aligned = ((size_t)(bptr) % 16) == 0;

    return vfpu_check(bptr) && aligned;
}
