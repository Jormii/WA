#pragma once

#include <math.h> // TODO: Remove

#include "vfpu.hpp"

#pragma region struct

union RGBA {
    u8 ptr[4];
    u32 rgba;
    V4<u8> v;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    struct {
        u8 r, g, b, a;
    };
#pragma GCC diagnostic pop

    V4f v4f() const;

    static RGBA from_v4f(const V4f &u);

    static RGBA mix(const RGBA &u, const RGBA &v, float t);
    static RGBA bary(                                //
        const RGBA &u, const RGBA &v, const RGBA &w, //
        float a, float b, float g                    //
    );
};

static_assert(sizeof(RGBA) == sizeof(u32));

using Texture = Buf2D<RGBA>;

RGBA texture_sample(const V2f &uv, const Texture &texture);
RGBA texture_sample(float u, float v, const Texture &texture);

struct PLight {
    V3f point;
    V4f color;
};

struct PLightS {
    V3f point;
    V4f color;
    M3f w;
    VFPU_ALIGNED M4f vp;
    Buf2D<float> depth_map;
};

#pragma endregion

#pragma region function

template <typename T>
V4<T> v4_point(const V3<T> &u);

template <typename T>
V4<T> v4_vector(const V3<T> &u);

template <typename T>
V3f persp_div(const V4<T> &u);

template <typename T>
M4f normal_m(const M4<T> &mv);

template <typename T>
M4<T> rotation_m(const V3<T> &x, const V3<T> &y, const V3<T> &z);

template <typename T>
M4<T> translation_m(const V3<T> &p);

template <typename T>
M4<T> translation_xyz_m(const T &x, const T &y, const T &z);

#pragma endregion

#pragma region template implementation

template <typename T>
V4<T> v4_point(const V3<T> &u) {
    return {u.x(), u.y(), u.z(), (T)1};
}

template <typename T>
V4<T> v4_vector(const V3<T> &u) {
    return {u.x(), u.y(), u.z(), (T)0};
}

template <typename T>
V3f persp_div(const V4<T> &u) {
    T w = u.w();
    MUST(!eq(w, (T)0));

    V3f persp_div_ = u.xyz() / (float)w;
    return persp_div_;
}

template <typename T>
M4f normal_m(const M4<T> &mv) {
    return mv.inverse().trans();
}

template <typename T>
M4<T> rotation_m(const V3<T> &x, const V3<T> &y, const V3<T> &z) {
#ifndef NDEBUG
    const V3f axes[3] = {x, y, z};
    for (i32 i = 0; i < 3; ++i) {
        float dot = V3<T>::dot(axes[i], axes[(i + 1) % 3]);

        i32 unit = eq(axes[i].mag(), 1.0f);
        i32 orthogonal = eq(dot, 0.0f);
        MUST(unit);
        MUST(orthogonal);
    }
#endif

    M4<T> rotation = {
        x.x(), x.y(), x.z(), 0, //
        y.x(), y.y(), y.z(), 0, //
        z.x(), z.y(), z.z(), 0, //
        0,     0,     0,     1, //
    };

    return rotation;
}

template <typename T>
M4<T> translation_m(const V3<T> &p) {
    return translation_xyz_m(p.x(), p.y(), p.z());
}

template <typename T>
M4<T> translation_xyz_m(const T &x, const T &y, const T &z) {
    M4<T> translation = {
        1, 0, 0, x, //
        0, 1, 0, y, //
        0, 0, 1, z, //
        0, 0, 0, 1, //
    };

    return translation;
}

#pragma endregion
