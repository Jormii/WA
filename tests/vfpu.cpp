#include <pspvfpu.h>

#include "cpp.hpp"

#include "vfpu.hpp"

static struct pspvfpu_context *vfpu_context = NULL;

i32 __VFPU_init_test(void) {
    vfpu_context = pspvfpu_initcontext();
    MUST(vfpu_context != NULL);

    return 1;
}

// vfpu.hpp::VFPU_ASSERT
i32 __VFPU_ASSERT_call(const i32 *ptr) {
    VFPU_ASSERT(ptr, 0);
    return 1;
}

i32 VFPU_ASSERT_ptr_is_NULL_test(void) {
    ASSERTZ(__VFPU_ASSERT_call(NULL) == 0);
    return 1;
}

// vfpu.hpp::VFPU_ALIGNED_ASSERT
i32 __VFPU_ALIGNED_ASSERT_call(const i32 *ptr) {
    VFPU_ALIGNED_ASSERT(ptr, 0);
    return 1;
}

i32 VFPU_ALIGNED_ASSERT_test(void) {
    VFPU_ALIGNED V4i v = V4i::zeros();

    ASSERTZ(__VFPU_ALIGNED_ASSERT_call(v.ptr));
    ASSERTZ(!__VFPU_ALIGNED_ASSERT_call(v.ptr + 1));
    ASSERTZ(!__VFPU_ALIGNED_ASSERT_call(v.ptr + 2));
    ASSERTZ(!__VFPU_ALIGNED_ASSERT_call(v.ptr + 3));
    ASSERTZ(__VFPU_ALIGNED_ASSERT_call(v.ptr + 4));

    return 1;
}

// vfpu.hpp::VFPU_LOAD_V4_ROW
// vfpu.hpp::VFPU_STORE_V4_ROW
i32 VFPU_LOAD_V4_ROW__and__VFPU_STORE_V4_ROW_test(void) {
#define MAT 0
#define ROW 0

    VFPU_ALIGNED V4i load = {1, 2, 3, 4};
    VFPU_LOAD_V4_ROW(MAT, ROW, load.ptr, 0);

    VFPU_ALIGNED V4i store = V4i::zeros();
    VFPU_STORE_V4_ROW(MAT, ROW, store.ptr, 0);
    ASSERTZ(store == load);

    return 1;

#undef MAT
#undef ROW
}

i32 __VFPU_deinit_test(void) {
    pspvfpu_deletecontext(vfpu_context);
    return 1;
}
