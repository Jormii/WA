#include <stdlib.h>

#include <pspdisplay.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspvfpu.h>

#include "vfpu.hpp"
#include "wa.hpp"

static const auto SETBUF = PSP_DISPLAY_SETBUF_NEXTFRAME;
static const auto PIXEL_FMT = PSP_DISPLAY_PIXEL_FORMAT_8888;

static i32 initialized = 0;
static struct pspvfpu_context *vfpu_context = NULL;

static RGBA *draw_buf = (RGBA *)DRAW_BUF_ADDR;
static RGBA *display_buf = (RGBA *)DISPLAY_BUF_ADDR;
static float *z_buf = NULL;

void VAO::ptr(i32 ptr_idx, void *ptr) {
    MUST(c_arr_idx_check(ptrs.ptr, ptrs.len, ptr_idx));
    MUST(ptr != NULL);

    ptrs[ptr_idx] = ptr;
}

void VAO::unif(i32 ptr_idx, i32 unif_idx, VAOType type) {
    MUST(c_arr_idx_check(ptrs.ptr, ptrs.len, ptr_idx));
    MUST(c_arr_idx_check(unifs.ptr, unifs.len, unif_idx));

    VAOUnif *unif = unifs + unif_idx;
    unif->ptr_idx = ptr_idx;
    unif->type = type;
}

void VAO::attr(                                                     //
    i32 ptr_idx, i32 attr_idx, i32 offset, i32 stride, VAOType type //
) {
    MUST(c_arr_idx_check(ptrs.ptr, ptrs.len, ptr_idx));
    MUST(c_arr_idx_check(attrs.ptr, attrs.len, attr_idx));
    MUST(offset >= 0);
    MUST(stride >= 0);

    VAOAttr *attr = attrs + attr_idx;
    attr->ptr_idx = ptr_idx;
    attr->offset = offset;
    attr->stride = stride;
    attr->type = type;
}

#define __UNIF_GET(RET_T, idx) *(RET_T *)(__get_unif(idx, VAOType::RET_T))

const M4f &VAO::unif_m4f(i32 unif_idx) const {
    return __UNIF_GET(M4f, unif_idx);
}

#undef __UNIF_GET

const void *VAO::__get_unif(i32 unif_idx, VAOType type) const {
    MUST(c_arr_idx_check(unifs.ptr, unifs.len, unif_idx));

    const VAOUnif *unif = unifs + unif_idx;
    i32 ptr_idx = unif->ptr_idx;

    MUST(c_arr_idx_check(ptrs.ptr, ptrs.len, ptr_idx))
    MUST(type == unif->type);

    const void *ptr = ptrs[ptr_idx];
    MUST(ptr != NULL);

    return ptr;
}

#define __ATTR_GET(RET_T, idx, v_idx)                                          \
    *(RET_T *)(__get_attr(idx, v_idx, VAOType::RET_T))

const V3f &VAO::attr_v3f(i32 attr_idx, i32 vertex_idx) const {
    return __ATTR_GET(V3f, attr_idx, vertex_idx);
}

const RGBA &VAO::attr_rgba(i32 attr_idx, i32 vertex_idx) const {
    return __ATTR_GET(RGBA, attr_idx, vertex_idx);
}

#undef __ATTR_GET

const void *VAO::__get_attr(i32 attr_idx, i32 vertex_idx, VAOType type) const {
    MUST(c_arr_idx_check(attrs.ptr, attrs.len, attr_idx));

    const VAOAttr *attr = attrs + attr_idx;
    i32 ptr_idx = attr->ptr_idx;
    i32 offset = attr->offset;
    i32 stride = attr->stride;

    MUST(c_arr_idx_check(ptrs.ptr, ptrs.len, ptr_idx));
    MUST(offset >= 0);
    MUST(stride >= 0);
    MUST(type == attr->type);

    const void *ptr = ptrs[ptr_idx];
    MUST(ptr != NULL);

    const u8 *out_ptr = ((u8 *)(ptr) + vertex_idx * stride) + offset;
    return out_ptr;
}

VAO VAO::alloc(i32 n_ptrs, i32 n_unifs, i32 n_attrs) {
    MUST(n_ptrs >= 0);
    MUST(n_unifs >= 0);
    MUST(n_attrs >= 0);

    i32 ptrs_size = n_ptrs * sizeof(void *);
    i32 unifs_size = n_unifs * sizeof(VAOUnif);
    i32 attrs_size = n_attrs * sizeof(VAOAttr);

    i32 size = ptrs_size + unifs_size + attrs_size;
    u8 *bptr = (u8 *)malloc(size); // TODO: Custom malloc()
    MUST(bptr != NULL);

    u8 *ptrs_ptr = bptr;
    u8 *unifs_ptr = ptrs_ptr + ptrs_size;
    u8 *attrs_ptr = unifs_ptr + unifs_size;

    Buf<void *> ptrs = {(void **)ptrs_ptr, n_ptrs};
    Buf<VAOUnif> unifs = {(VAOUnif *)unifs_ptr, n_unifs};
    Buf<VAOAttr> attrs = {(VAOAttr *)attrs_ptr, n_attrs};

    return {bptr, ptrs, unifs, attrs};
}

i32 wa_init() {
    UNTESTED("i32 wa_init()");
    ASSERTZ(!initialized);

    z_buf = (float *)malloc(FRAME_BUF_SIZE * sizeof(float));
    ASSERTZ(z_buf != NULL);

    vfpu_context = pspvfpu_initcontext();
    ASSERTZ(vfpu_context != NULL);

    sceDisplaySetMode(0, SCREEN_WIDTH, SCREEN_HEIGHT);
    sceDisplaySetFrameBuf(display_buf, FRAME_BUF_WIDTH, PIXEL_FMT, SETBUF);

    initialized = 1;
    return 1;
}

void wa_clear(RGBA color) {
#define CLEAR_MAT 0
#define CLEAR_RGBA_ROW 0
#define CLEAR_Z_ROW 1

    UNTESTED("void wa_clear(RGBA color)");

    prof_kick(SLOT_WA_CLEAR);

    RGBA *rgba_ptr = draw_buf;
    float *z_ptr = z_buf;

    i32 c = color.rgba;
    float z = CANONICAL_Z_CLEAR;
    VFPU_ALIGNED i32 rgba_row[] = {c, c, c, c};
    VFPU_ALIGNED float z_row[] = {z, z, z, z};
    VFPU_LOAD_V4_ROW(CLEAR_MAT, CLEAR_RGBA_ROW, rgba_row, 0);
    VFPU_LOAD_V4_ROW(CLEAR_MAT, CLEAR_Z_ROW, z_row, 0);

    for (i32 _ = 0; _ < FRAME_BUF_SIZE; _ += 16, rgba_ptr += 16, z_ptr += 16) {
        VFPU_STORE_V4_ROW(CLEAR_MAT, CLEAR_RGBA_ROW, rgba_ptr, 0);
        VFPU_STORE_V4_ROW(CLEAR_MAT, CLEAR_RGBA_ROW, rgba_ptr, 16);
        VFPU_STORE_V4_ROW(CLEAR_MAT, CLEAR_RGBA_ROW, rgba_ptr, 32);
        VFPU_STORE_V4_ROW(CLEAR_MAT, CLEAR_RGBA_ROW, rgba_ptr, 48);

        VFPU_STORE_V4_ROW(CLEAR_MAT, CLEAR_Z_ROW, z_ptr, 0);
        VFPU_STORE_V4_ROW(CLEAR_MAT, CLEAR_Z_ROW, z_ptr, 16);
        VFPU_STORE_V4_ROW(CLEAR_MAT, CLEAR_Z_ROW, z_ptr, 32);
        VFPU_STORE_V4_ROW(CLEAR_MAT, CLEAR_Z_ROW, z_ptr, 48);
    }

    prof_stop(SLOT_WA_CLEAR);

#undef CLEAR_MAT
#undef CLEAR_RGBA_ROW
#undef CLEAR_Z_ROW
}

void wa_swap_bufs() {
    SWAP(draw_buf, display_buf);
    sceDisplaySetFrameBuf(display_buf, FRAME_BUF_WIDTH, PIXEL_FMT, SETBUF);
}

i32 wa_buf_in(float x, float y) {
    UNTESTED("i32 wa_buf_in(float x, float y)");

    i32 in = x >= VIEWPORT_LEFT && x <= VIEWPORT_RIGHT //
             && y >= VIEWPORT_TOP && y <= VIEWPORT_BOTTOM;

    return in;
}

i32 wa_buf_idx(float x, float y) {
    UNTESTED("i32 wa_buf_idx(float x, float y)");
    MUST(wa_buf_in(x, y));

    i32 idx = round(x + 0.5f) + round(y + 0.5f) * FRAME_BUF_WIDTH;
    MUST(idx >= 0 && idx < FRAME_BUF_SIZE);

    return idx;
}

V3f wa_up() {
    UNTESTED("V3f wa_up()");
    return {0.0f, 1.0f, 0.0f};
}

M3f wa_viewport() {
    UNTESTED("M3f wa_viewport()");

    float w2 = 0.5f * (SCREEN_WIDTH - 1);
    float h2 = 0.5f * (SCREEN_HEIGHT - 1);

    M3f m = {
        w2, 0,   w2 - 0.5f, //
        0,  -h2, h2 - 0.5f, //
        0,  0,   1,         //
    };

    return m;
}

M4f wa_look_at(V3f eye, V3f at, V3f up) {
    UNTESTED("M4f wa_look_at(V3f eye, V3f at, V3f up)");

    MUST(eye != at);
    MUST(!eq(up.mag(), 0.0f));

    V3f z = eye - at;
    V3f x = V3f::cross(up, z);
    V3f y = V3f::cross(z, x);

    x = x.norm();
    y = y.norm();
    z = z.norm();

    VFPU_ALIGNED M4f r = rotation_m(x, y, z);
    VFPU_ALIGNED M4f t = translation_m(-eye);
    M4f look_at = r * t;

    return look_at;
}

M4f wa_orthographic(float l, float r, float b, float t, float n, float f) {
    UNTESTED("M4f wa_orthographic(...)");

    MUST(l < r);
    MUST(b < t);
    MUST(n > 0 && n < f);

    V3f p = {
        -(r + l) / (r - l),
        -(t + b) / (t - b),
        -(f + n) / (f - n),
    };

    M4f orthographic = translation_m(p);
    orthographic.get(0, 0) = 2.0f / (r - l);
    orthographic.get(1, 1) = 2.0f / (t - b);
    orthographic.get(2, 2) = -2.0f / (f - n);

    return orthographic;
}

M4f wa_perspective(float l, float r, float b, float t, float n, float f) {
    UNTESTED("M4f wa_perspective(...)")

    MUST(l < r);
    MUST(b < t);
    MUST(n > 0 && n < f);

    M4f perspective = M4f::zeros();
    perspective.get(0, 0) = (2.0f * n) / (r - l);
    perspective.get(0, 2) = (r + l) / (r - l);
    perspective.get(1, 1) = (2.0f * n) / (t - b);
    perspective.get(1, 2) = (t + b) / (t - b);
    perspective.get(2, 2) = -(f + n) / (f - n);
    perspective.get(2, 3) = (-2.0f * f * n) / (f - n);
    perspective.get(3, 2) = -1;

    return perspective;
}

M4f wa_perspective_fov(float fov, float n, float f) {
    UNTESTED("M4f wa_perspective_fov(float fov, float n, float f)")

    MUST(fov > 0 && fov < M_PI_2);
    MUST(n > 0 && n < f);

    float t = n * tanf(0.5f * fov);
    float r = ((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT) * t;

    return wa_perspective(-r, r, -t, t, n, f);
}

void wa_render(                                      //
    const VAO &vao, const Buf<V3i> triangles,        //
    VertexSh_fp vertex_sh, FragmentSh_fp fragment_sh //
) {
    UNTESTED("void wa_render()")

    MUST(vertex_sh != NULL);
    MUST(fragment_sh != NULL);

    prof_kick(SLOT_WA_RENDER);

    M3f w = wa_viewport();

    for (i32 i = 0; i < triangles.len; ++i) {
        V3f screen[3];
        RGBA screen_colors[3];
        const V3i &triangle = triangles[i];

        for (i32 j = 0; j < triangle.len(); ++j) {
            i32 vertex_idx = triangle[j];
            VertexShOut out = vertex_sh(vertex_idx, vao);

            V3f canonical = persp_div(out.vertex);
            V3f canonical_xy = {canonical.x(), canonical.y(), 1};

            screen[j] = w * canonical_xy;
            screen_colors[j] = out.color;

            screen[j].z() = canonical.z();
        }

        float x0, y0;
        float xf, yf;
        x0 = xf = screen[0].x();
        y0 = yf = screen[0].y();
        for (i32 j = 1; j < 3; ++j) {
            x0 = min(x0, screen[j].x());
            xf = max(xf, screen[j].x());
            y0 = min(y0, screen[j].y());
            yf = max(yf, screen[j].y());
        }

        x0 = max(x0, VIEWPORT_LEFT);
        xf = min(xf, VIEWPORT_RIGHT);
        y0 = max(y0, VIEWPORT_TOP);
        yf = min(yf, VIEWPORT_BOTTOM);

        for (float y = y0; y <= yf; ++y) {
            for (float x = x0; x <= xf; ++x) {
                const V3f &sx = screen[0];
                const V3f &sy = screen[1];
                const V3f &sz = screen[2];

                float alpha =
                    wa_fline(x, y, sy.x(), sy.y(), sz.x(), sz.y()) /
                    wa_fline(sx.x(), sx.y(), sy.x(), sy.y(), sz.x(), sz.y());
                float beta =
                    wa_fline(x, y, sz.x(), sz.y(), sx.x(), sx.y()) /
                    wa_fline(sy.x(), sy.y(), sz.x(), sz.y(), sx.x(), sx.y());
                float gamma = 1 - (alpha + beta);

                if (alpha >= 0 && beta >= 0 && gamma >= 0) {
                    i32 idx = wa_buf_idx(x, y);

                    float z = CANONICAL_Z_CLEAR;
                    bary_v(                        //
                        &sx.z(), &sy.z(), &sz.z(), //
                        alpha, beta, gamma,        //
                        &z, 1                      //
                    );

                    if (z < CANONICAL_Z_MAX || z_buf[idx] <= z) {
                        continue;
                    }

                    RGBA color = RGBA::bary(                                  //
                        screen_colors[0], screen_colors[1], screen_colors[2], //
                        alpha, beta, gamma                                    //
                    );

                    FragmentShOut out = fragment_sh(color);

                    draw_buf[idx] = out.color;
                    z_buf[idx] = z;
                }
            }
        }
    }

    prof_stop(SLOT_WA_RENDER);
}

float wa_fline(float x, float y, float px, float py, float qx, float qy) {
    UNTESTED("float wa_fline(...)")
    return (py - qy) * x + (qx - px) * y + px * qy - qx * py;
}

void wa_draw_line(float x0, float y0, float xf, float yf, RGBA c0, RGBA cf) {
    UNTESTED("void wa_draw_line(...)");

    V2f p = {x0, y0};
    V2f q = {xf, yf};
    float dx = abs(xf - x0);
    float dy = abs(yf - y0);

    i32 argmax = dx <= dy;
    if (q.ptr[argmax] < p.ptr[argmax]) {
        SWAP(p, q);
        SWAP(c0, cf);
    }

    V2f v = q - p;
    i32 argmin = !argmax;
    float e0 = p.ptr[argmax];
    float ef = q.ptr[argmax];
    for (float e = e0; e <= ef; ++e) {
        float t = (e - e0) / v.ptr[argmax];
        float e_argmin = p.ptr[argmin] + t * v.ptr[argmin];

        V2f pixel;
        pixel.ptr[argmax] = e;
        pixel.ptr[argmin] = e_argmin;

        if (wa_buf_in(pixel.x(), pixel.y())) {
            RGBA color = RGBA::mix(c0, cf, t);
            i32 idx = wa_buf_idx(pixel.x(), pixel.y());

            draw_buf[idx] = color;
            z_buf[idx] = CANONICAL_Z_MAX;
        }
    }
}
