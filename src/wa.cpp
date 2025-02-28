#include <pspdisplay.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspvfpu.h>

#include "vfpu.hpp"
#include "wa.hpp"

static const auto SETBUF = PSP_DISPLAY_SETBUF_NEXTFRAME;
static const auto PIXEL_FMT = PSP_DISPLAY_PIXEL_FORMAT_8888;

static i32 initialized = 0;
static struct pspvfpu_context *vfpuContext = NULL;

// TODO: Arbitrary addresses?
static RGBA *draw_buf = (RGBA *)DRAW_BUF_ADDR;
static RGBA *display_buf = (RGBA *)DISPLAY_BUF_ADDR;

i32 wa_init() {
    ASSERTZ(!initialized);

    vfpuContext = pspvfpu_initcontext();
    ASSERTZ(vfpuContext != NULL);

    sceDisplaySetMode(0, SCREEN_WIDTH, SCREEN_HEIGHT);
    sceDisplaySetFrameBuf(display_buf, FRAME_BUF_WIDTH, PIXEL_FMT, SETBUF);

    initialized = 1;
    return 1;
}

void wa_clear(RGBA color) {
#define CLEAR_MAT 0
#define CLEAR_ROW 0

    RGBA *ptr = draw_buf;
    const RGBA *END = ptr + FRAME_BUF_SIZE;

    i32 c = color.rgba;
    VFPU_ALIGNED i32 row[] = {c, c, c, c};
    VFPU_LOAD_V4_ROW(CLEAR_MAT, CLEAR_ROW, row, 0);

    for (; ptr != END; ptr += 16) {
        VFPU_STORE_V4_ROW(CLEAR_MAT, CLEAR_ROW, ptr, 0);
        VFPU_STORE_V4_ROW(CLEAR_MAT, CLEAR_ROW, ptr, 16);
        VFPU_STORE_V4_ROW(CLEAR_MAT, CLEAR_ROW, ptr, 32);
        VFPU_STORE_V4_ROW(CLEAR_MAT, CLEAR_ROW, ptr, 48);
    }

#undef CLEAR_MAT
#undef CLEAR_ROW
}

void wa_swap_bufs() {
    SWAP(draw_buf, display_buf);
    sceDisplaySetFrameBuf(display_buf, FRAME_BUF_WIDTH, PIXEL_FMT, SETBUF);
}

i32 wa_buf_in(float x, float y) {
    i32 in = x >= VIEWPORT_LEFT && x <= VIEWPORT_RIGHT //
             && y >= VIEWPORT_TOP && y <= VIEWPORT_BOTTOM;

    return in;
}

i32 wa_buf_idx(float x, float y) {
    MUST(wa_buf_in(x, y));

    i32 idx = round(x + 0.5f) + round(y + 0.5f) * FRAME_BUF_WIDTH;
    MUST(idx >= 0 && idx < FRAME_BUF_SIZE);

    return idx;
}

V3f wa_up() { return {.v = {0.0f, 1.0f, 0.0f}}; }

M3f wa_viewport() {
    float w2 = (SCREEN_WIDTH - 1) / 2.0f;
    float h2 = (SCREEN_HEIGHT - 1) / 2.0f;

    // clang-format off
    M3f m = {.m = {
        w2,     0,      w2 - 0.5f,
        0,      -h2,    h2 - 0.5f,
        0,      0,      1,
    }};
    // clang-format on

    return m;
}

M4f wa_look_at(V3f eye, V3f at, V3f up) {
    MUST(eye != at);
    MUST(!eq(up.mag(), 0.0f));

    V3f z = eye - at;
    V3f x = V3f::cross(up, z);
    V3f y = V3f::cross(z, x);

    x.normalize();
    y.normalize();
    z.normalize();

    M4f r = M4f::rotation(x, y, z);
    M4f t = M4f::translation_xyz(-eye.x, -eye.y, -eye.z);
    M4f look_at = M4f::mmult(r, t);

    return look_at;
}

M4f wa_orthographic(float l, float r, float b, float t, float n, float f) {
    MUST(l < r);
    MUST(b < t);
    MUST(n > 0 && n < f);

    V3f p = {
        .x = -(r + l) / (r - l),
        .y = -(t + b) / (t - b),
        .z = -(f + n) / (f - n),
    };

    M4f orthographic = M4f::translation(p);
    orthographic.rows[0].v[0] = 2.0f / (r - l);
    orthographic.rows[1].v[1] = 2.0f / (t - b);
    orthographic.rows[2].v[2] = -2.0f / (f - n);

    return orthographic;
}

M4f wa_perspective(float l, float r, float b, float t, float n, float f) {
    MUST(l < r);
    MUST(b < t);
    MUST(n > 0 && n < f);

    M4f perspective = M4f::zeros();
    perspective.rows[0].v[0] = (2.0f * n) / (r - l);
    perspective.rows[0].v[2] = (r + l) / (r - l);
    perspective.rows[1].v[1] = (2.0f * n) / (t - b);
    perspective.rows[1].v[2] = (t + b) / (t - b);
    perspective.rows[2].v[2] = -(f + n) / (f - n);
    perspective.rows[2].v[3] = (-2.0f * f * n) / (f - n);
    perspective.rows[3].v[2] = -1;

    return perspective;
}

M4f wa_perspective_fov(float fov, float n, float f) {
    MUST(fov > 0 && fov < M_PI_2);

    float t = n * tanf(0.5f * fov);
    float r = ((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT) * t;

    return wa_perspective(-r, r, -t, t, n, f);
}

void wa_draw_line(float x0, float y0, float xf, float yf, RGBA c0, RGBA cf) {
    V2f p = {.v = {x0, y0}};
    V2f q = {.v = {xf, yf}};
    float dx = abs(xf - x0);
    float dy = abs(yf - y0);

    i32 argmax = dx <= dy;
    if (q.v[argmax] < p.v[argmax]) {
        SWAP(p, q);
        SWAP(c0, cf);
    }

    V2f v = q - p;
    i32 argmin = !argmax;
    float e0 = p.v[argmax];
    float ef = q.v[argmax];
    for (float e = e0; e <= ef; ++e) {
        float t = (e - e0) / v.v[argmax];
        float e_argmin = p.v[argmin] + t * v.v[argmin];

        V2f pixel;
        pixel.v[argmax] = e;
        pixel.v[argmin] = e_argmin;

        if (wa_buf_in(pixel.x, pixel.y)) {
            RGBA color = RGBA::mix(c0, cf, t);
            i32 idx = wa_buf_idx(pixel.x, pixel.y);

            draw_buf[idx] = color;
        }
    }
}
