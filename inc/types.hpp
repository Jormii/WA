#pragma once

#include "cpp.hpp"

union RGBA {
    u8 v[4];
    u32 rgba;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    struct {
        u8 r, g, b, a;
    };
#pragma GCC diagnostic pop

    static RGBA mix(RGBA u, RGBA v, float t);
};

template <typename T> union V2 {
    T v[2];
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    struct {
        T x, y;
    };

    static V2 sub(V2 u, V2 v) { return {.x = u.x - v.x, .y = u.y - v.y}; }
};

using V2f = V2<float>;

i32 round(float x);

template <typename T> T abs(T x) { return (x >= 0) ? x : -x; }
