#pragma once

#include "cpp.hpp"

typedef union RGBA {
    u8 v[4];
    u32 rgba;
    struct {
        u8 r, g, b, a;
    } channels;
} RGBA;

i32 wa_init();
void wa_clear(RGBA color);
void wa_swap_bufs();
