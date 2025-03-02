#include "types.hpp"

#pragma region Structs

RGBA RGBA::mix(RGBA u, RGBA v, float t) {
    if (t <= 0) {
        return u;
    } else if (t >= 1) {
        return v;
    }

    RGBA mix;
    for (i32 i = 0; i < 4; ++i) {
        float c = (1.0f - t) * (float)(u.v[i]) + t * (float)(v.v[i]);
        mix.v[i] = c;
    }

    return mix;
}

RGBA RGBA::mix_bary(RGBA u, RGBA v, RGBA w, float a, float b, float g) {
    if (a >= 1) {
        return u;
    } else if (b >= 1) {
        return v;
    } else if (g >= 1) {
        return w;
    }

    RGBA mix;
    bary_v(u.v, v.v, w.v, a, b, g, mix.v, 4);

    return mix;
}

#pragma endregion

#pragma region Functions

/*
i32 round(float x) {
    float offset = (x >= 0) ? 0.5f : -0.5f;
    i32 round = (i32)(x + offset);

    return round;
}
*/

#pragma endregion