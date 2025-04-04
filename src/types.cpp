#include "types.hpp"

#pragma region struct

V4f RGBA::v4f() const { return v / 255.0f; }

RGBA RGBA::from_v4f(const V4f &u) {
    u8 g = (u8)(clamp(roundf(u.y() * 255.0f), 0.0f, 255.0f));
    u8 r = (u8)(clamp(roundf(u.x() * 255.0f), 0.0f, 255.0f));
    u8 b = (u8)(clamp(roundf(u.z() * 255.0f), 0.0f, 255.0f));
    u8 a = (u8)(clamp(roundf(u.w() * 255.0f), 0.0f, 255.0f));

    RGBA rgba = {r, g, b, a};
    return rgba;
}

RGBA RGBA::mix(const RGBA &u, const RGBA &v, float t) {
    UNTESTED("RGBA RGBA::mix(const RGBA &u, const RGBA &v, float t)");
    V4f mixf = V4<u8>::mix(u.v, v.v, t);
    RGBA mix = {.v = mixf.cast<u8>()};

    return mix;
}

RGBA RGBA::bary(                                 //
    const RGBA &u, const RGBA &v, const RGBA &w, //
    float a, float b, float g                    //
) {
    UNTESTED("RGBA RGBA::bary(...)");
    V4f baryf = V4<u8>::bary(u.v, v.v, w.v, a, b, g);
    RGBA bary = {.v = baryf.cast<u8>()};

    return bary;
}

RGBA texture_sample(const V2f &uv, const Texture &texture) {
    return texture_sample(uv.x(), uv.y(), texture);
}

RGBA texture_sample(float u, float v, const Texture &texture) {
    MUST(u >= 0);
    MUST(u <= 1);
    MUST(v >= 0);
    MUST(v <= 1);

    i32 i = roundf((1 - v) * texture.rows);
    i32 j = roundf(u * texture.cols);

    if (i >= texture.rows) {
        i -= texture.rows;
    }
    if (j >= texture.cols) {
        j -= texture.cols;
    }

    RGBA color = texture.get(i, j);
    return color;
}

#pragma endregion
