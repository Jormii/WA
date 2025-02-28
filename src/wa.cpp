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
    return x >= VIEWPORT_LEFT && x <= VIEWPORT_RIGHT //
           && y >= VIEWPORT_TOP && y <= VIEWPORT_BOTTOM;
}

i32 wa_buf_idx(float x, float y) {
    MUST(wa_buf_in(x, y));

    i32 idx = round(x + 0.5f) + round(y + 0.5f) * FRAME_BUF_WIDTH;
    MUST(idx >= 0 && idx < FRAME_BUF_SIZE);

    return idx;
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

    i32 argmin = !argmax;
    V2f v = V2f::sub(q, p);
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
