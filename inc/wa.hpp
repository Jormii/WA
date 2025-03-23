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

#define CANONICAL_Z_MAX -1.0f
#define CANONICAL_Z_CLEAR 1.0f
#define RGBA_Z_MAX 0
#define RGBA_Z_CLEAR 255

#define VERTEX_SH_STUB(NAME) VertexShOut NAME(const VertexShIn *in)
#define FRAGMENT_SH_STUB(NAME) FragmentShOut NAME(const FragmentShIn *in)

enum ProfSlots {
    SLOT_LOOP,
    SLOT_WA_CLEAR,
    SLOT_WA_RENDER,
};

enum class VAOType {
    V3f,
    M4f,
    RGBA,
};

struct VAOUnif {
    i32 ptr_idx;
    VAOType type;
};

struct VAOAttr {
    i32 ptr_idx;
    i32 offset;
    i32 stride;
    VAOType type;
};

struct VAO {
    const u8 *bptr;
    Buf<void *> ptrs;
    Buf<VAOUnif> unifs;
    Buf<VAOAttr> attrs;

    void ptr(i32 ptr_idx, void *ptr);
    void unif(i32 ptr_idx, i32 unif_idx, VAOType type);
    void attr(i32 ptr_idx, i32 attr_idx, i32 offset, i32 stride, VAOType type);

    const M4f &unif_m4f(i32 unif_idx) const;
    const void *__get_unif(i32 unif_idx, VAOType type) const;

    const V3f &attr_v3f(i32 attr_idx, i32 vertex_idx) const;
    const RGBA &attr_rgba(i32 attr_idx, i32 vertex_idx) const;
    const void *__get_attr(i32 attr_idx, i32 vertex_idx, VAOType type) const;

    static VAO alloc(i32 n_ptrs, i32 n_unifs, i32 n_attrs);
};

struct VertexShOut {
    V4f position;
    RGBA color;
};

struct FragmentShIn {
    const RGBA &color;
};

struct FragmentShOut {
    RGBA color;
};

typedef VertexShOut (*VertexSh_fp)(const VertexShIn &in);
typedef FragmentShOut (*FragmentSh_fp)(const FragmentShIn &in);

[[nodiscard]] i32 wa_init();
void wa_clear(RGBA color);
void wa_swap_bufs();

[[nodiscard]] i32 wa_buf_in(float x, float y);
i32 wa_buf_idx(float x, float y);

V3f wa_up();
M3f wa_viewport();
M4f wa_look_at(V3f eye, V3f at, V3f up);
M4f wa_orthographic(float l, float r, float b, float t, float n, float f);
M4f wa_perspective(float l, float r, float b, float t, float n, float f);
M4f wa_perspective_fov(float fov, float n, float f);

void wa_render(                                      //
    const M4f &m, const M4f &v, const M4f &p,        //
    Buf<V3f> vs, Buf<RGBA> cs, Buf<V3i> ts,          //
    VertexSh_fp vertex_sh, FragmentSh_fp fragment_sh //
);

float wa_fline(float x, float y, float px, float py, float qx, float qy);

void wa_draw_line(float x0, float y0, float xf, float yf, RGBA c0, RGBA cf);

VertexShOut wa_vertex_sh_basic(const VertexShIn &in);
FragmentShOut wa_fragment_sh_basic(const FragmentShIn &in);
