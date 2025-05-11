#pragma once

#include "cpp.hpp"

#pragma region macro

#define VFPU_ALIGNED __attribute__((aligned(16)))

#define VFPU_V4 "q"

// NOTE: PPSSPP's debugger flips rows and columns.
//       To simplify debugging VFPU_R and VFPU_C are swapped as well.
#define VFPU_M(MAT) "M" #MAT "00"
#define VFPU_C(MAT, ROW, COL) "R" #MAT #COL #ROW
#define VFPU_R(MAT, ROW, COL) "C" #MAT #ROW #COL

#define VFPU_OP_LOAD "lv"
#define VFPU_OP_STORE "sv"

#define VFPU_OP_MUL_MM "vmmul"

#define VFPU_INST_MEMORY(OP_CODE, DIM, REG, ptr, OFFSET)                       \
    MUST(vfpu_check(ptr));                                                     \
    asm(OP_CODE "." DIM " " REG ", " #OFFSET "(%0)" : : "r"(ptr) : "memory");

#define VFPU_INST_BINARY(OP_CODE, SIZE, DST, SRC1, SRC2)                       \
    asm(OP_CODE "." SIZE " " DST ", " SRC1 ", " SRC2 ";");

#define VFPU_LOAD_V4_ROW(MAT, ROW, ptr, OFFSET)                                \
    MUST(vfpu_aligned_check(ptr, OFFSET));                                     \
    VFPU_INST_MEMORY(VFPU_OP_LOAD, VFPU_V4, VFPU_R(MAT, ROW, 0), ptr, OFFSET);

#define VFPU_STORE_V4_ROW(MAT, ROW, ptr, OFFSET)                               \
    MUST(vfpu_aligned_check(ptr, OFFSET));                                     \
    VFPU_INST_MEMORY(VFPU_OP_STORE, VFPU_V4, VFPU_R(MAT, ROW, 0), ptr, OFFSET);

#define VFPU_LOAD_M4(MAT, ptr, OFFSET)                                         \
    VFPU_LOAD_V4_ROW(MAT, 0, ptr, OFFSET + 0);                                 \
    VFPU_LOAD_V4_ROW(MAT, 1, ptr, OFFSET + 16);                                \
    VFPU_LOAD_V4_ROW(MAT, 2, ptr, OFFSET + 32);                                \
    VFPU_LOAD_V4_ROW(MAT, 3, ptr, OFFSET + 48);

#define VFPU_STORE_M4(MAT, ptr, OFFSET)                                        \
    VFPU_STORE_V4_ROW(MAT, 0, ptr, OFFSET + 0);                                \
    VFPU_STORE_V4_ROW(MAT, 1, ptr, OFFSET + 16);                               \
    VFPU_STORE_V4_ROW(MAT, 2, ptr, OFFSET + 32);                               \
    VFPU_STORE_V4_ROW(MAT, 3, ptr, OFFSET + 48);

// clang-format off
#define VFPU_MUL_M4(DST_MAT, SRC1_MAT, SRC2_MAT)                             \
    NOTE("VFPU uses row vectors => Swap SRC1 and SRC2")                       \
    VFPU_INST_BINARY(VFPU_OP_MUL_MM, VFPU_V4, VFPU_M(DST_MAT), VFPU_M(SRC2_MAT), VFPU_M(SRC1_MAT))
// clang-format on

#pragma endregion

#pragma region function

[[nodiscard]] i32 vfpu_check(const void *ptr);
[[nodiscard]] i32 vfpu_aligned_check(const void *ptr, i32 offset);

template <>
void mul_mm<4, float>(const float *m, const float *a, float *out);

#pragma endregion

#pragma region template implementation

template <>
inline Mat<4, float> Mat<4, float>::operator*(const Mat &rhs) const {
    VFPU_ALIGNED Mat<4, float> mul_mm_;
    mul_mm<4, float>(ptr, rhs.ptr, mul_mm_.ptr);

    return mul_mm_;
}

template <>
inline void mul_mm<4, float>(const float *m, const float *a, float *out) {
#define M_MAT 0
#define A_MAT 1
#define OUT_MAT 2

    MUST(m != NULL);
    MUST(a != NULL);
    MUST(out != NULL);

    VFPU_LOAD_M4(M_MAT, m, 0);
    VFPU_LOAD_M4(A_MAT, a, 0);
    VFPU_MUL_M4(OUT_MAT, M_MAT, A_MAT);
    VFPU_STORE_M4(OUT_MAT, out, 0);

#undef M_MAT
#undef A_MAT
#undef OUT_MAT
}

#pragma endregion

#ifndef PPSSPP
#define TAG()
#else
#define TAG() asm("vmzero.p M000;");
#endif
