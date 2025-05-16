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

enum ProfSlots {
    SLOT_LOOP,
    SLOT_WA_CLEAR,
    SLOT_WA_RENDER,
    SLOT_WA_RENDER_SHADOW,
};

enum class VAOType {
    V2f,
    V3f,
    V4f,
    M4f,
    RGBA,

    Texture,
    PLight,
    PLightS,
};

enum class FrontFace {
    CW,
    CCW,
    BACKFACE,
};

struct Bone {
    i32 parent_idx;
    VFPU_ALIGNED M4f m_inv;            // Model
    VFPU_ALIGNED M4f transform;        // Local (respective to parent)
    VFPU_ALIGNED M4f transform_global; // Updated by {Skeleton::update()}

    static Bone init_I(i32 parent_idx, const V3f &position);
};

struct VAOBuf {
    void *ptr;
    i32 len;
    // TODO: error: forming reference to void T &Buf<T>::operator[](i32 idx)
};

struct VAOIn {
    i32 buf_idx;
    i32 offset;
    i32 stride;
    VAOType type;
};

struct VAOUnif {
    i32 buf_idx;
    VAOType type;
};

struct VAOOut {
    i32 offset;
    VAOType type;
};

struct VAO {
    const u8 *const bptr; // Pointer returned by malloc()
    Buf<VAOBuf> bufs;
    Buf<VAOIn> ins;
    Buf<VAOUnif> unifs;
    i32 outs_stride;
    Buf<u8> outs_buf;
    Buf<u8> outs_bary_buf; // TODO: Might be possible to reuse {outs_buf}
    Buf<VAOOut> outs;

    void buf(i32 buf_idx, void *ptr, i32 len);
    void in(i32 buf_idx, i32 in_idx, i32 offset, i32 stride, VAOType type);
    void unif(i32 buf_idx, i32 unif_idx, VAOType type);
    void __make_bary(float alpha, float beta, float gamma) const;

    const V2f &in_v2f(i32 in_idx, i32 v_idx) const;
    const V3f &in_v3f(i32 in_idx, i32 v_idx) const;
    const RGBA &in_rgba(i32 in_idx, i32 v_idx) const;
    const void *__get_in(i32 in_idx, i32 v_idx, VAOType type) const;

    const Buf<V3f> &unif_v3f(i32 unif_idx) const;
    const Buf<M4f> &unif_m4f(i32 unif_idx) const;
    const Buf<RGBA> &unif_rgba(i32 unif_idx) const;
    const Buf<Texture> &unif_texture(i32 unif_idx) const;
    const Buf<PLight> &unif_p_light(i32 unif_idx) const;
    const Buf<PLightS> &unif_p_light_s(i32 unif_idx) const;
    const VAOBuf &__get_unif(i32 unif_idx, VAOType type) const;

    V2f &out_v2f(i32 out_idx, i32 tri_v_idx) const;
    V3f &out_v3f(i32 out_idx, i32 tri_v_idx) const;
    V4f &out_v4f(i32 out_idx, i32 tri_v_idx) const;
    RGBA &out_rgba(i32 out_idx, i32 tri_v_idx) const;
    void *__get_out(i32 out_idx, i32 tri_v_idx, VAOType type) const;

    const V2f &out_bary_v2f(i32 out_idx) const;
    const V3f &out_bary_v3f(i32 out_idx) const;
    const V4f &out_bary_v4f(i32 out_idx) const;
    const RGBA &out_bary_rgba(i32 out_idx) const;
    void *__get_out_bary(i32 out_idx, VAOType type) const;

    static i32 size(VAOType type);
    static VAO alloc(i32 n_bufs, i32 n_ins, i32 n_unifs, Buf<VAOType> outs_ts);
};

struct VertexShOut {
    V4f vertex;
};

struct FragmentShOut {
    V4f color;
    i32 discard;
};

typedef VertexShOut (*VertexSh_fp)(i32 v_idx, i32 tri_v_idx, const VAO &vao);
typedef FragmentShOut (*FragmentSh_fp)(const VAO &vao);

typedef V4f (*ShadowSh_fp)(                   //
    i32 v_idx, i32 tri_v_idx, const VAO &vao, //
    const PLightS &light                      //
);

[[nodiscard]] i32 wa_init();
void wa_clear(RGBA color);
void wa_clear_depth(float *ptr, i32 len);
void wa_swap_bufs();

[[nodiscard]] i32 wa_buf_in(float x, float y);
[[nodiscard]] i32 wa_buf_in(float x, float y, i32 rows, i32 cols);
i32 wa_buf_idx(float x, float y);
i32 wa_buf_idx(float x, float y, i32 rows, i32 cols);

V3f wa_up();
M3f wa_viewport(i32 rows, i32 cols);
M4f wa_look_at(V3f eye, V3f at, V3f up);
M4f wa_orthographic(float l, float r, float b, float t, float n, float f);
M4f wa_perspective(float l, float r, float b, float t, float n, float f);
M4f wa_perspective_fov(float fov, float n, float f);

M4f wa_rot_x(float angle);
M4f wa_rot_y(float angle);
M4f wa_rot_z(float angle);
M4f wa_translate(const V3f &point);
M4f wa_translate_inv(const V3f &point);

M4f wa_rot_x_and_translate(float angle, const V3f &point);
M4f wa_rot_y_and_translate(float angle, const V3f &point);
M4f wa_rot_z_and_translate(float angle, const V3f &point);

void wa_render(                                                       //
    const VAO &vao, const Buf<V3i> triangles,                         //
    FrontFace front, VertexSh_fp vertex_sh, FragmentSh_fp fragment_sh //
);
void wa_render_shadow(                                     //
    const VAO &vao, const Buf<V3i> triangles,              //
    FrontFace front, ShadowSh_fp shadow_sh, PLightS &light //
);

float wa_fline(float x, float y, float px, float py, float qx, float qy);

void wa_draw_line(float x0, float y0, float xf, float yf, RGBA c0, RGBA cf);
