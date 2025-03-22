#include "vfpu.hpp"

i32 vfpu_check(const void *ptr, i32 offset) {
    // TODO: Check offset?
    return ptr != NULL;
}

i32 vfpu_aligned_check(const void *ptr, i32 offset) {
    i32 aligned = ((size_t)(ptr) % 16) == 0;
    return vfpu_check(ptr, offset) && aligned;
}