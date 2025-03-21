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
    VFPU_ALIGNED i32 u[4] = {2, 3, 4, 5};

    ASSERTZ(__VFPU_ALIGNED_ASSERT_call(u + 0) == 1);
    ASSERTZ(__VFPU_ALIGNED_ASSERT_call(u + 1) == 0);
    ASSERTZ(__VFPU_ALIGNED_ASSERT_call(u + 2) == 0);
    ASSERTZ(__VFPU_ALIGNED_ASSERT_call(u + 3) == 0);
    ASSERTZ(__VFPU_ALIGNED_ASSERT_call(u + 4) == 1);

    return 1;
}

// vfpu.hpp::VFPU_LOAD_V4_ROW
// vfpu.hpp::VFPU_STORE_V4_ROW
i32 VFPU_LOAD_V4_ROW__and__VFPU_STORE_V4_ROW_test(void) {
#define MAT 0
#define ROW 0

    VFPU_ALIGNED i32 load[4] = {2, 3, 4, 5};
    VFPU_ALIGNED i32 store[C_ARR_LEN(load)];

    VFPU_LOAD_V4_ROW(MAT, ROW, load, 0);
    VFPU_STORE_V4_ROW(MAT, ROW, store, 0);
    ASSERTZ(eq_v(store, load, C_ARR_LEN(load)));

    return 1;

#undef MAT
#undef ROW
}

// vfpu.hpp::VFPU_LOAD_M4
// vfpu.hpp::VFPU_STORE_M4
i32 VFPU_LOAD_M4__and__VFPU_STORE_M4_test(void) {
#define MAT 0

    VFPU_ALIGNED i32 load[4 * 4] = {
        2, 3, 4, 5, //
        3, 4, 5, 6, //
        4, 5, 6, 7, //
        5, 6, 7, 8, //
    };
    VFPU_ALIGNED i32 store[C_ARR_LEN(load)];

    VFPU_LOAD_M4(MAT, load, 0);
    VFPU_STORE_M4(MAT, store, 0);
    ASSERTZ(eq_v(store, load, C_ARR_LEN(load)));

    return 1;

#undef MAT
}

// cpp.hpp::Mat
i32 Mat_operator_mult_Mat__4_float_test(void) {
    VFPU_ALIGNED Mat<4, float> m = {
        2, 3, 4, 5, //
        3, 4, 5, 6, //
        4, 5, 6, 7, //
        5, 6, 7, 8, //
    };
    VFPU_ALIGNED Mat<4, float> a = {
        1, 2, 3, 4, //
        4, 3, 2, 1, //
        6, 7, 8, 9, //
        9, 8, 7, 6, //
    };
    VFPU_ALIGNED Mat<4, float> expected_mmult = {
        83,  81,  79,  77,  //
        103, 101, 99,  97,  //
        123, 121, 119, 117, //
        143, 141, 139, 137, //
    };

    Mat<4, float> mmult = m * a;
    ASSERTZ(mmult == expected_mmult);

    return 1;
}

// cpp.hpp::(Functions)
i32 mmult_m__4_float_test(void) {
    VFPU_ALIGNED float m[4 * 4] = {
        2, 3, 4, 5, //
        3, 4, 5, 6, //
        4, 5, 6, 7, //
        5, 6, 7, 8, //
    };
    VFPU_ALIGNED float a[C_ARR_LEN(m)] = {
        1, 2, 3, 4, //
        4, 3, 2, 1, //
        6, 7, 8, 9, //
        9, 8, 7, 6, //
    };
    VFPU_ALIGNED float expected_mmult[C_ARR_LEN(m)] = {
        83,  81,  79,  77,  //
        103, 101, 99,  97,  //
        123, 121, 119, 117, //
        143, 141, 139, 137, //
    };

    VFPU_ALIGNED float mmult[C_ARR_LEN(m)];
    const float *out = mmult_m<4, float>(m, a, mmult);
    ASSERTZ(out == mmult);
    ASSERTZ(eq_v(mmult, expected_mmult, C_ARR_LEN(m)));

    return 1;
}

i32 __VFPU_deinit_test(void) {
    pspvfpu_deletecontext(vfpu_context);
    return 1;
}
