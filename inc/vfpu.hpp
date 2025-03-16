#pragma once

#include "cpp.hpp"

#define VFPU_ALIGNED __attribute__((aligned(16)))

#define VFPU_V4 "q"

// NOTE: PPSSPP's debugger flips rows and columns.
//       To simplify debugging VFPU_R and VFPU_C are swapped as well.
#define VFPU_M(MAT) "M" #MAT "00"
#define VFPU_C(MAT, ROW, COL) "R" #MAT #ROW #COL
#define VFPU_R(MAT, ROW, COL) "C" #MAT #ROW #COL

#define VFPU_OP_LOAD "lv"
#define VFPU_OP_STORE "sv"

#define VFPU_OP_MMULT_M "vmmul"

// TODO: Assert OFFSET?
#define VFPU_ASSERT(ptr, OFFSET)                                               \
    TESTED();                                                                  \
    ASSERTZ(ptr != NULL);

#define VFPU_ALIGNED_ASSERT(ptr, OFFSET)                                       \
    TESTED();                                                                  \
    VFPU_ASSERT(ptr, OFFSET);                                                  \
    ASSERTZ(((size_t)(ptr) % 16) == 0);

#define VFPU_INST_MEMORY(OP_CODE, DIM, REG, ptr, OFFSET)                       \
    NOTE("LEFT UNTESTED");                                                     \
    VFPU_ASSERT(ptr, OFFSET);                                                  \
    asm(OP_CODE "." DIM " " REG ", " #OFFSET "(%0)" : : "r"(ptr) : "memory");

#define VFPU_INST_BINARY(OP_CODE, SIZE, DST, SRC1, SRC2)                       \
    NOTE("LEFT UNTESTED");                                                     \
    asm(OP_CODE "." SIZE " " DST ", " SRC1 ", " SRC2 ";");

#define VFPU_LOAD_V4_ROW(MAT, ROW, ptr, OFFSET)                                \
    TESTED();                                                                  \
    VFPU_ALIGNED_ASSERT(ptr, OFFSET);                                          \
    VFPU_INST_MEMORY(VFPU_OP_LOAD, VFPU_V4, VFPU_R(MAT, ROW, 0), ptr, OFFSET);

#define VFPU_STORE_V4_ROW(MAT, ROW, ptr, OFFSET)                               \
    TESTED();                                                                  \
    VFPU_ALIGNED_ASSERT(ptr, OFFSET);                                          \
    VFPU_INST_MEMORY(VFPU_OP_STORE, VFPU_V4, VFPU_R(MAT, ROW, 0), ptr, OFFSET);

#define VFPU_LOAD_M4(MAT, ptr, OFFSET)                                         \
    TESTED();                                                                  \
    VFPU_LOAD_V4_ROW(MAT, 0, ptr, OFFSET + 0);                                 \
    VFPU_LOAD_V4_ROW(MAT, 1, ptr, OFFSET + 16);                                \
    VFPU_LOAD_V4_ROW(MAT, 2, ptr, OFFSET + 32);                                \
    VFPU_LOAD_V4_ROW(MAT, 3, ptr, OFFSET + 48);

#define VFPU_STORE_M4(MAT, ptr, OFFSET)                                        \
    TESTED();                                                                  \
    VFPU_STORE_V4_ROW(MAT, 0, ptr, OFFSET + 0);                                \
    VFPU_STORE_V4_ROW(MAT, 1, ptr, OFFSET + 16);                               \
    VFPU_STORE_V4_ROW(MAT, 2, ptr, OFFSET + 32);                               \
    VFPU_STORE_V4_ROW(MAT, 3, ptr, OFFSET + 48);

// clang-format off
#define VFPU_MMULT_M4(DST_MAT, SRC1_MAT, SRC2_MAT)                             \
    NOTE("LEFT UNTESTED");                                                     \
    NOTE("VFPU uses row vectors => Swap SRC1 and SRC2");                       \
    VFPU_INST_BINARY(VFPU_OP_MMULT_M, VFPU_V4, VFPU_M(DST_MAT), VFPU_M(SRC2_MAT), VFPU_M(SRC1_MAT))
// clang-format on

template <>
inline float *mmult_m<4, float>(const float *m, const float *a, float *out) {
#define M_MAT 0
#define A_MAT 1
#define OUT_MAT 2

    TESTED();
    ASSERTZ(m != NULL);
    ASSERTZ(a != NULL);
    ASSERTZ(out != NULL);

    VFPU_LOAD_M4(M_MAT, m, 0);
    VFPU_LOAD_M4(A_MAT, a, 0);
    VFPU_MMULT_M4(OUT_MAT, M_MAT, A_MAT);
    VFPU_STORE_M4(OUT_MAT, out, 0);

    return out;

#undef M_MAT
#undef A_MAT
#undef OUT_MAT
}
