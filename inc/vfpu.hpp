#pragma once

#define VFPU_ALIGNED __attribute__((aligned(16)))

#define VFPU_V4 "q"

// NOTE: PPSSPP's debugger flips rows and columns.
//       To simplify debugging VFPU_R and VFPU_C are swapped as well.
#define VFPU_C(MAT, ROW, COL) "R" #MAT #COL #ROW
#define VFPU_R(MAT, ROW, COL) "C" #MAT #COL #ROW

#define VFPU_OP_LOAD "lv"
#define VFPU_OP_STORE "sv"

#define VFPU_INST_MEMORY(OP_CODE, DIM, REG, ptr, OFFSET)                       \
    MUST(ptr != NULL)                                                          \    
    asm(OP_CODE "." DIM " " REG ", " #OFFSET "(%0)" : : "r"(ptr) : "memory");

// TODO: Assert OFFSET?
// TODO: Assert alignment
#define VFPU_LOAD_V4_ROW(MAT, ROW, ptr, OFFSET)                                \
    VFPU_INST_MEMORY(VFPU_OP_LOAD, VFPU_V4, VFPU_R(MAT, ROW, 0), ptr, OFFSET);

// TODO: Assert OFFSET?
// TODO: Assert alignment
#define VFPU_STORE_V4_ROW(MAT, ROW, ptr, OFFSET)                               \
    VFPU_INST_MEMORY(VFPU_OP_STORE, VFPU_V4, VFPU_R(MAT, ROW, 0), ptr, OFFSET);
