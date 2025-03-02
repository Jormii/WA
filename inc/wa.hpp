#pragma once

#include "types.hpp"

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 272
#define FRAME_BUF_WIDTH 512
#define FRAME_BUF_SIZE (SCREEN_HEIGHT * FRAME_BUF_WIDTH)

#define DRAW_BUF_ADDR 0x04300000
#define DISPLAY_BUF_ADDR 0x04000000

#define VIEWPORT_LEFT -0.5f
#define VIEWPORT_RIGHT (SCREEN_WIDTH - 1 - 0.5f)
#define VIEWPORT_TOP -0.5f
#define VIEWPORT_BOTTOM (SCREEN_HEIGHT - 1 - 0.5f)

#define VERTEX_SH_STUB(NAME)                                                   \
    VertexShOut NAME(V3f v, V2f uv, RGBA c, const M4f &mvp)
#define FRAGMENT_SH_STUB(NAME) FragmentShOut NAME(V2f uv, RGBA c)

struct VertexShIn {
    V3f vertex;
    V2f uv;
    RGBA color;

    const M4f &mvp;
};

struct VertexShOut {
    V4f position;
    RGBA color;
};

struct FragmentShIn {
    V2f uv;
    RGBA color;
};

struct FragmentShOut {
    RGBA color;
};

typedef VertexShOut (*VertexSh_fp)(const VertexShIn &in);
typedef FragmentShOut (*FragmentSh_fp)(const FragmentShIn &in);

i32 wa_init();
void wa_clear(RGBA color);
void wa_swap_bufs();

i32 wa_buf_in(float x, float y);
i32 wa_buf_idx(float x, float y);

V3f wa_up();
M3f wa_viewport();
M4f wa_look_at(V3f eye, V3f at, V3f up);
M4f wa_orthographic(float l, float r, float b, float t, float n, float f);
M4f wa_perspective(float l, float r, float b, float t, float n, float f);
M4f wa_perspective_fov(float fov, float n, float f);

// clang-format off
void wa_render(
    const M4f &m, const M4f &v, const M4f &p,
    BufC<V3f> vs, BufC<V2f> uvs, BufC<RGBA> cs, BufC<V3i> ts,
    VertexSh_fp vertex_sh, FragmentSh_fp fragment_sh
);
// clang-format on

float wa_fline(float x, float y, float px, float py, float qx, float qy);

void wa_draw_line(float x0, float y0, float xf, float yf, RGBA c0, RGBA cf);

VertexShOut wa_vertex_sh_basic(const VertexShIn &in);
FragmentShOut wa_fragment_sh_basic(const FragmentShIn &in);
