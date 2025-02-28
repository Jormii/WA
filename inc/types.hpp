#pragma once

#include <cmath> // TODO (Jorge): Remove

#include "cpp.hpp"

#pragma region Structs

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

template <typename T, i32 N = 2>
union V2 {
    static_assert(N == 2);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    T v[N];
    struct {
        T x, y;
    };
#pragma GCC diagnostic pop

    V2 operator-(const V2 &rhs) const;
};

template <typename T, i32 N = 3>
union V3 {
    static_assert(N == 3);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    T v[N];
    struct {
        T x, y, z;
    };
#pragma GCC diagnostic pop

    float mag() const;
    V3 &normalize();

    static T dot(V3 u, V3 v);
    static V3 cross(V3 u, V3 v);

    V3 operator-(const V3 &rhs) const;
    bool operator==(const V3 &rhs) const;
};

template <typename T, i32 N = 4>
union V4 {
    static_assert(N == 4);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    T v[N];
    struct {
        T x, y, z, w;
    };
#pragma GCC diagnostic pop

    static T dot(V4 u, V4 v);
};

using V2f = V2<float>;
using V3f = V3<float>;
using V4f = V4<float>;
static_assert(sizeof(V2f) == 2 * sizeof(float));
static_assert(sizeof(V3f) == 3 * sizeof(float));
static_assert(sizeof(V4f) == 4 * sizeof(float));

template <typename T, i32 N = 3>
union M3 {
    static_assert(N == 3);

    T m[N * N];
    V3<T> rows[N];
};

template <typename T, i32 N = 4>
union M4 {
    static_assert(N == 4);

    T m[N * N];
    V4<T> rows[N];

    M4 transposed() const;

    static M4 zeros();
    static M4 mmult(const M4 &a, const M4 &b);
    static M4 rotation(V3<T> x, V3<T> y, V3<T> z);
    static M4 translation(V3<T> p);
    static M4 translation_xyz(float x, float y, float z);
};

using M3f = M3<float>;
using M4f = M4<float>;
static_assert(sizeof(M3f) == 3 * 3 * sizeof(float));
static_assert(sizeof(M4f) == 4 * 4 * sizeof(float));

#pragma endregion

#pragma region Functions

// i32 round(float x);

// template <typename T> T abs(T x);

template <typename T>
i32 eq(T x, T y);

template <typename T>
float mag_v(const T *u, i32 len);

template <typename T>
T dot_v(const T *u, const T *v, i32 len);

template <typename T>
i32 eq_v(const T *u, const T *v, i32 len);

template <typename T>
void normalize_v(const T *u, T *out, i32 len);

template <typename T>
void sub_v(const T *u, const T *v, float *out, i32 len);

#pragma endregion

#pragma region T_IMPL

template <typename T, i32 N>
V2<T, N> V2<T, N>::operator-(const V2 &rhs) const {
    V2 out;
    sub_v(v, rhs.v, out.v, N);
    return out;
}

template <typename T, i32 N>
float V3<T, N>::mag() const {
    return mag_v(v, N);
}

template <typename T, i32 N>
V3<T, N> &V3<T, N>::normalize() {
    normalize_v(v, v, N);
    return *this;
};

template <typename T, i32 N>
T V3<T, N>::dot(V3 u, V3 v) {
    return dot_v(u.v, v.v, N);
}

template <typename T, i32 N>
V3<T, N> V3<T, N>::cross(V3 u, V3 v) {
    return {
        .x = u.y * v.z - u.z * v.y,
        .y = u.z * v.x - u.x * v.z,
        .z = u.x * v.y - u.y * v.x,
    };
}

template <typename T, i32 N>
V3<T, N> V3<T, N>::operator-(const V3 &rhs) const {
    V3 out;
    sub_v(v, rhs.v, out.v, N);
    return out;
}

template <typename T, i32 N>
bool V3<T, N>::operator==(const V3 &rhs) const {
    return eq_v(v, rhs.v, N);
}

template <typename T, i32 N>
T V4<T, N>::dot(V4 u, V4 v) {
    return dot_v(u.v, v.v, N);
}

template <typename T, i32 N>
M4<T, N> M4<T, N>::transposed() const {
    M4 t;
    for (i32 r = 0; r < N; ++r) {
        for (i32 c = 0; c < N; ++c) {
            t.rows[c].v[r] = rows[r].v[c];
        }
    }

    return t;
}

template <typename T, i32 N>
M4<T, N> M4<T, N>::zeros() {
    M4 m;
    for (i32 i = 0; i < N * N; ++i) {
        m.m[i] = 0;
    }

    return m;
}

template <typename T, i32 N>
M4<T, N> M4<T, N>::mmult(const M4 &a, const M4 &b) {
    M4 mmult;
    M4 bt = b.transposed();

    i32 idx = 0;
    for (i32 r = 0; r < N; ++r) {
        for (i32 c = 0; c < N; ++c) {
            const V4<T> &row = a.rows[r];
            const V4<T> &column = bt.rows[c];
            mmult.m[idx++] = V4<T>::dot(row, column);
        }
    }

    return mmult;
}

template <typename T, i32 N>
M4<T, N> M4<T, N>::rotation(V3<T> x, V3<T> y, V3<T> z) {
#ifndef NDEBUG
    const V3f axes[3] = {x, y, z};
    for (i32 i = 0; i < 3; ++i) {
        float dot = V3<T>::dot(axes[i], axes[(i + 1) % 3]);

        i32 orthogonal = eq(dot, 0.0f);
        i32 unit = eq(axes[i].mag(), 1.0f);
        MUST(orthogonal && unit);
    }
#endif

    // clang-format off
            M4 rotation = {.m = {
                x.x,    x.y,    x.z,    0,
                y.x,    y.y,    y.z,    0,
                z.x,    z.y,    z.z,    0,
                0,      0,      0,      1,
            }};
    // clang-format on

    return rotation;
}

template <typename T, i32 N>
M4<T, N> M4<T, N>::translation(V3<T> p) {
    return M4::translation_xyz(p.x, p.y, p.z);
}

template <typename T, i32 N>
M4<T, N> M4<T, N>::translation_xyz(float x, float y, float z) {
    // clang-format off
    M4 translation = {.m = {
        1, 0, 0, x,
        0, 1, 0, y,
        0, 0, 1, z,
        0, 0, 0, 1,
    }};
    // clang-format on

    return translation;
}

#pragma endregion

#pragma region Tf_IMPL

/*
template <typename T> T abs(T x) {
    T abs = (x >= 0) ? x : -x;
    return abs;
}
*/

template <typename T>
inline i32 eq(T x, T y) {
    i32 eq = x == y;
    return eq;
}

template <>
inline i32 eq<float>(float x, float y) {
    float d = x - y;
    float abs_ = abs(d);
    i32 nearly_eq = abs_ <= __FLT_EPSILON__;

    return nearly_eq;
}

template <typename T>
float mag_v(const T *u, i32 len) {
    MUST(u != NULL);
    MUST(len >= 0);

    T dot = dot_v(u, u, len);
    float mag = sqrtf((float)dot);

    return mag;
}

template <typename T>
T dot_v(const T *u, const T *v, i32 len) {
    MUST(u != NULL);
    MUST(v != NULL);
    MUST(len >= 0);

    T dot = 0;
    for (i32 i = 0; i < len; ++i) {
        dot += u[i] * v[i];
    }

    return dot;
}

template <typename T>
i32 eq_v(const T *u, const T *v, i32 len) {
    MUST(u != NULL);
    MUST(v != NULL);
    MUST(len >= 0);

    i32 eq_ = 1;
    for (i32 i = 0; i < len && eq_; ++i) {
        eq_ &= eq(u[i], v[i]);
    }

    return eq_;
}

template <typename T>
void normalize_v(const T *u, T *out, i32 len) {
    MUST(u != NULL);
    MUST(out != NULL);
    MUST(len >= 0);

    float mag = mag_v(u, len);
    for (i32 i = 0; i < len; ++i) {
        out[i] = u[i] / mag;
    }
}

template <typename T>
void sub_v(const T *u, const T *v, float *out, i32 len) {
    MUST(u != NULL);
    MUST(v != NULL);
    MUST(out != NULL);
    MUST(len >= 0);

    for (i32 i = 0; i < len; ++i) {
        out[i] = u[i] - v[i];
    }
}

#pragma endregion
