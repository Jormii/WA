#include "types.hpp"

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

i32 round(float x) {
    float offset = (x >= 0) ? 0.5f : -0.5f;
    return (i32)(x + offset);
}
