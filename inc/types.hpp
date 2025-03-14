#pragma once

#include <cmath> // TODO (Jorge): Remove

#include "cpp.hpp"

#pragma region struct

union RGBA {
    u8 ptr[4];
    u32 rgba;
    V4<u8> v;

    static RGBA mix(const RGBA &u, const RGBA &v, float t);
    static RGBA bary(                                //
        const RGBA &u, const RGBA &v, const RGBA &w, //
        float a, float b, float g                    //
    );
};

static_assert(sizeof(RGBA) == sizeof(u32));

#pragma endregion

#pragma region function

template <typename T>
V3f persp_div(const V4<T> &u);

template <typename T>
M4<T> rotation_m(const V3<T> &x, const V3<T> &y, const V3<T> &z);

template <typename T>
M4<T> translation_m(const V3<T> &p);

template <typename T>
M4<T> translation_xyz_m(const T &x, const T &y, const T &z);

#pragma endregion

#pragma region template implementation

template <typename T>
V3f persp_div(const V4<T> &u) {
    T w = u.w();
    if (!eq(w, 0.0f)) {
        return u.xyz() / w;
    } else {
        return u.xyz().template cast<float>(); // BRUH
    }
}

template <typename T>
M4<T> rotation_m(const V3<T> &x, const V3<T> &y, const V3<T> &z) {
#ifndef NDEBUG
    const V3f axes[3] = {x, y, z};
    for (i32 i = 0; i < 3; ++i) {
        float dot = V3<T>::dot(axes[i], axes[(i + 1) % 3]);

        i32 orthogonal = eq(dot, 0.0f);
        i32 unit = eq(axes[i].mag(), 1.0f);
        MUST(orthogonal);
        MUST(unit);
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
