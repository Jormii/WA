#include "types.hpp"

#pragma region struct

RGBA RGBA::mix(const RGBA &u, const RGBA &v, float t) {
    V4f mixf = V4<u8>::mix(u.v, v.v, t);

    RGBA mix = {.v = mixf.cast<u8>()};
    return mix;
}

RGBA RGBA::bary(                                 //
    const RGBA &u, const RGBA &v, const RGBA &w, //
    float a, float b, float g                    //
) {
    V4f baryf = V4<u8>::bary(u.v, v.v, w.v, a, b, g);

    RGBA bary = {.v = baryf.cast<u8>()};
    return bary;
}

#pragma endregion
