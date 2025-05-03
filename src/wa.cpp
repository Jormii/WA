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

void VAO::buf(i32 buf_idx, void *ptr, i32 len) {
    MUST(c_arr_idx_check(bufs.ptr, bufs.len, buf_idx));
    MUST(c_arr_check(ptr, len));

    VAOBuf *buf = bufs + buf_idx;
    buf->ptr = ptr;
    buf->len = len;
}

void VAO::in(i32 buf_idx, i32 in_idx, i32 offset, i32 stride, VAOType type) {
    MUST(c_arr_idx_check(bufs.ptr, bufs.len, buf_idx));
    MUST(c_arr_idx_check(ins.ptr, ins.len, in_idx));
    MUST(offset >= 0);
    MUST(stride > 0);

    VAOIn *in = ins + in_idx;
    in->buf_idx = buf_idx;
    in->offset = offset;
    in->stride = stride;
    in->type = type;
}

void VAO::unif(i32 buf_idx, i32 unif_idx, VAOType type) {
    MUST(c_arr_idx_check(bufs.ptr, bufs.len, buf_idx));
    MUST(c_arr_idx_check(unifs.ptr, unifs.len, unif_idx));

    VAOUnif *unif = unifs + unif_idx;
    unif->buf_idx = buf_idx;
    unif->type = type;
}

void VAO::__make_bary(float alpha, float beta, float gamma) const {
    for (i32 out_idx = 0; out_idx < outs.len; ++out_idx) {
        const VAOOut *out = outs + out_idx;

        VAOType type = out->type;
        void *ptr = __get_out_bary(out_idx, type);
        const void *a = __get_out(out_idx, 0, type);
        const void *b = __get_out(out_idx, 1, type);
        const void *g = __get_out(out_idx, 2, type);

        switch (type) {
        case VAOType::V2f:
            *((V2f *)ptr) = V2f::bary(                 //
                *((V2f *)a), *((V2f *)b), *((V2f *)g), //
                alpha, beta, gamma                     //
            );
            break;
        case VAOType::V3f:
            *((V3f *)ptr) = V3f::bary(                 //
                *((V3f *)a), *((V3f *)b), *((V3f *)g), //
                alpha, beta, gamma                     //
            );
            break;
        case VAOType::V4f:
            *((V4f *)ptr) = V4f::bary(                 //
                *((V4f *)a), *((V4f *)b), *((V4f *)g), //
                alpha, beta, gamma                     //
            );
            break;
        case VAOType::RGBA:
            *((RGBA *)ptr) = RGBA::bary(                  //
                *((RGBA *)a), *((RGBA *)b), *((RGBA *)g), //
                alpha, beta, gamma                        //
            );
            break;
        default:
            fprintf(stderr, "%ld\n", (i32)type);
            MUST(0 && (i32)type);
            break;
        }
    }
}

#define __IN_GET(RET_T, idx, v_idx)                                            \
    *(RET_T *)(__get_in(idx, v_idx, VAOType::RET_T))

const V2f &VAO::in_v2f(i32 in_idx, i32 v_idx) const {
    return __IN_GET(V2f, in_idx, v_idx);
}

const V3f &VAO::in_v3f(i32 in_idx, i32 v_idx) const {
    return __IN_GET(V3f, in_idx, v_idx);
}

const RGBA &VAO::in_rgba(i32 in_idx, i32 v_idx) const {
    return __IN_GET(RGBA, in_idx, v_idx);
}

#undef __IN_GET

const void *VAO::__get_in(i32 in_idx, i32 v_idx, VAOType type) const {
    MUST(c_arr_idx_check(ins.ptr, ins.len, in_idx));

    const VAOIn *in = ins + in_idx;
    i32 buf_idx = in->buf_idx;
    i32 offset = in->offset;
    i32 stride = in->stride;

    MUST(c_arr_idx_check(bufs.ptr, bufs.len, buf_idx));
    MUST(offset >= 0);
    MUST(stride > 0);
    MUST(type == in->type);

    VAOBuf buf = bufs[buf_idx];
    MUST(c_arr_idx_check(buf.ptr, buf.len, v_idx));

    const u8 *out_ptr = ((u8 *)(buf.ptr) + v_idx * stride) + offset;
    return out_ptr;
}

#define __UNIF_GET(RET_T, idx) (Buf<RET_T> &)(__get_unif(idx, VAOType::RET_T))

const Buf<V3f> &VAO::unif_v3f(i32 unif_idx) const {
    return __UNIF_GET(V3f, unif_idx);
};

const Buf<M4f> &VAO::unif_m4f(i32 unif_idx) const {
    return __UNIF_GET(M4f, unif_idx);
}

const Buf<RGBA> &VAO::unif_rgba(i32 unif_idx) const {
    return __UNIF_GET(RGBA, unif_idx);
}

const Buf<Texture> &VAO::unif_texture(i32 unif_idx) const {
    return __UNIF_GET(Texture, unif_idx);
}

const Buf<PLight> &VAO::unif_p_light(i32 unif_idx) const {
    return __UNIF_GET(PLight, unif_idx);
}

const Buf<PLightS> &VAO::unif_p_light_s(i32 unif_idx) const {
    return __UNIF_GET(PLightS, unif_idx);
}

#undef __UNIF_GET

const VAOBuf &VAO::__get_unif(i32 unif_idx, VAOType type) const {
    MUST(c_arr_idx_check(unifs.ptr, unifs.len, unif_idx));

    const VAOUnif *unif = unifs + unif_idx;
    i32 buf_idx = unif->buf_idx;

    MUST(c_arr_idx_check(bufs.ptr, bufs.len, buf_idx))
    MUST(type == unif->type);

    const VAOBuf &buf = bufs[buf_idx];
    return buf;
}

#define __OUT_GET(RET_T, idx, t_v_idx)                                         \
    *(RET_T *)(__get_out(idx, t_v_idx, VAOType::RET_T))

V2f &VAO::out_v2f(i32 out_idx, i32 tri_v_idx) const {
    return __OUT_GET(V2f, out_idx, tri_v_idx);
}

V3f &VAO::out_v3f(i32 out_idx, i32 tri_v_idx) const {
    return __OUT_GET(V3f, out_idx, tri_v_idx);
}

V4f &VAO::out_v4f(i32 out_idx, i32 tri_v_idx) const {
    return __OUT_GET(V4f, out_idx, tri_v_idx);
}

RGBA &VAO::out_rgba(i32 out_idx, i32 tri_v_idx) const {
    return __OUT_GET(RGBA, out_idx, tri_v_idx);
}

#undef __OUT_GET

void *VAO::__get_out(i32 out_idx, i32 tri_v_idx, VAOType type) const {
    MUST(c_arr_idx_check(outs.ptr, outs.len, out_idx));
    MUST(tri_v_idx >= 0);
    MUST(tri_v_idx < 3);

    const VAOOut *out = outs + out_idx;
    i32 offset = out->offset;

    MUST(offset >= 0);
    MUST(type == out->type);

    u8 *out_ptr = (outs_buf.ptr + tri_v_idx * outs_stride) + offset;
    return out_ptr;
}

#define __OUT_BARY_GET(RET_T, idx)                                             \
    *(RET_T *)(__get_out_bary(idx, VAOType::RET_T))

const V2f &VAO::out_bary_v2f(i32 out_idx) const {
    return __OUT_BARY_GET(V2f, out_idx);
}

const V3f &VAO::out_bary_v3f(i32 out_idx) const {
    return __OUT_BARY_GET(V3f, out_idx);
}

const V4f &VAO::out_bary_v4f(i32 out_idx) const {
    return __OUT_BARY_GET(V4f, out_idx);
}

const RGBA &VAO::out_bary_rgba(i32 out_idx) const {
    return __OUT_BARY_GET(RGBA, out_idx);
}

#undef __OUT_BARY_GET

void *VAO::__get_out_bary(i32 out_idx, VAOType type) const {
    MUST(c_arr_idx_check(outs.ptr, outs.len, out_idx));

    const VAOOut *out = outs + out_idx;
    i32 offset = out->offset;

    MUST(offset >= 0);
    MUST(type == out->type);

    u8 *out_ptr = outs_bary_buf.ptr + offset;
    return out_ptr;
}

i32 VAO::size(VAOType type) {
    switch (type) {
    case VAOType::V2f:
        return sizeof(V2f);
    case VAOType::V3f:
        return sizeof(V3f);
    case VAOType::V4f:
        return sizeof(V4f);
    case VAOType::M4f:
        return sizeof(M4f);
    case VAOType::RGBA:
        return sizeof(RGBA);
    default:
        fprintf(stderr, "%ld\n", (i32)type);
        MUST(0 && (i32)type);
        return 0;
    }
}

VAO VAO::alloc(i32 n_bufs, i32 n_ins, i32 n_unifs, Buf<VAOType> outs_ts) {
    MUST(n_bufs >= 0);
    MUST(n_ins >= 0);
    MUST(n_unifs >= 0);
    MUST(c_arr_check(outs_ts.ptr, outs_ts.len));

    i32 n_outs = outs_ts.len;

    i32 outs_ts_size = 0;
    for (i32 i = 0; i < n_outs; ++i) {
        VAOType type = outs_ts[i];
        outs_ts_size += VAO::size(type);
    }

    i32 bufs_size = n_bufs * sizeof(VAOBuf);
    i32 ins_size = n_ins * sizeof(VAOIn);
    i32 unifs_size = n_unifs * sizeof(VAOUnif);
    i32 outs_buf_size = 3 * outs_ts_size;
    i32 outs_bary_buf_size = outs_ts_size;
    i32 outs_size = n_outs * sizeof(VAOOut);

    i32 size = bufs_size + ins_size + unifs_size //
               + outs_buf_size + outs_bary_buf_size + outs_size;

    u8 *bptr = (u8 *)malloc(size); // TODO: Custom malloc()
    MUST(bptr != NULL);

    u8 *bufs_ptr = bptr;
    u8 *ins_ptr = bufs_ptr + bufs_size;
    u8 *unifs_ptr = ins_ptr + ins_size;
    u8 *outs_buf_ptr = unifs_ptr + unifs_size;
    u8 *outs_bary_buf_ptr = outs_buf_ptr + outs_buf_size;
    u8 *outs_ptr = outs_bary_buf_ptr + outs_bary_buf_size;

    Buf<VAOBuf> bufs = {(VAOBuf *)bufs_ptr, n_bufs};
    Buf<VAOIn> ins = {(VAOIn *)ins_ptr, n_ins};
    Buf<VAOUnif> unifs = {(VAOUnif *)unifs_ptr, n_unifs};
    Buf<u8> outs_buf = {(u8 *)outs_buf_ptr, outs_buf_size};
    Buf<u8> outs_bary_buf = {(u8 *)outs_bary_buf_ptr, outs_bary_buf_size};
    Buf<VAOOut> outs = {(VAOOut *)outs_ptr, n_outs};

    i32 offset = 0;
    for (i32 i = 0; i < n_outs; ++i) {
        VAOOut *out = outs + i;
        VAOType type = outs_ts[i];

        out->offset = offset;
        out->type = type;

        offset += VAO::size(type);
    }

    i32 outs_stride = offset;
    return {bptr, bufs, ins, unifs, outs_stride, outs_buf, outs_bary_buf, outs};
}

i32 wa_init() {
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

void wa_clear_depth(float *ptr, i32 len) {
    MUST(c_arr_check(ptr, len));

    float z = CANONICAL_Z_CLEAR;
    for (i32 i = 0; i < len; ++i) {
        ptr[i] = z;
    }
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

i32 wa_buf_in(float x, float y, i32 rows, i32 cols) {
    MUST(rows >= 0);
    MUST(cols >= 0);

    float rows_ = (float)(rows - 1) - 0.5f;
    float cols_ = (float)(cols - 1) - 0.5f;

    i32 in = x >= -0.5f && x <= cols_ //
             && y >= -0.5f && y <= rows_;
    return in;
}

i32 wa_buf_idx(float x, float y) {
    MUST(wa_buf_in(x, y));

    i32 idx = round(x + 0.5f) + round(y + 0.5f) * FRAME_BUF_WIDTH;
    MUST(idx >= 0 && idx < FRAME_BUF_SIZE);

    return idx;
}

i32 wa_buf_idx(float x, float y, i32 rows, i32 cols) {
    MUST(rows >= 0);
    MUST(cols >= 0);
    MUST(wa_buf_in(x, y, rows, cols));

    i32 idx = round(x + 0.5f) + round(y + 0.5f) * cols;
    MUST(idx >= 0 && idx < (rows * cols));

    return idx;
}

V3f wa_up() { return {0.0f, 1.0f, 0.0f}; }

M3f wa_viewport() {
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
    MUST(fov > 0 && fov < M_PI_2);
    MUST(n > 0 && n < f);

    float t = n * tanf(0.5f * fov);
    float r = ((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT) * t;

    return wa_perspective(-r, r, -t, t, n, f);
}

void wa_render(                                                            //
    const VAO &vao, const Buf<V3i> triangles,                              //
    FrontFace front_face, VertexSh_fp vertex_sh, FragmentSh_fp fragment_sh //
) {
    MUST(vertex_sh != NULL);
    MUST(fragment_sh != NULL);

    prof_kick(SLOT_WA_RENDER);

    M3f w = wa_viewport();

    for (i32 i = 0; i < triangles.len; ++i) {
        V3f canonical[3];
        const V3i &triangle = triangles[i];

        for (i32 tri_v_idx = 0; tri_v_idx < triangle.len(); ++tri_v_idx) {
            i32 v_idx = triangle[tri_v_idx];
            VertexShOut out = vertex_sh(v_idx, tri_v_idx, vao);

            canonical[tri_v_idx] = persp_div(out.vertex);
        }

        i32 cull = 0;
        switch (front_face) {
        case FrontFace::CW: {
            V3f u = canonical[1] - canonical[0];
            V3f v = canonical[2] - canonical[1];
            V3f cross = V3f::cross(u, v);
            float dot = -cross.z(); // NOTE: dot(cross, {0,0,-1})
            cull = dot <= 0;
        } break;
        case FrontFace::CCW: {
            V3f u = canonical[2] - canonical[0];
            V3f v = canonical[1] - canonical[2];
            V3f cross = V3f::cross(u, v);
            float dot = -cross.z(); // NOTE: dot(cross, {0,0,-1})
            cull = dot <= 0;
        } break;
        case FrontFace::BACKFACE:
            break;
        default:
            MUST(0 && (i32)front_face);
        }

        if (cull) {
            continue;
        }

        V3f screen[3];
        for (i32 i = 0; i < 3; ++i) {
            V3f vertex = canonical[i];
            V3f vertex_xy1 = {vertex.x(), vertex.y(), 1};

            screen[i] = w * vertex_xy1;
            screen[i].z() = vertex.z();
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

                    vao.__make_bary(alpha, beta, gamma);
                    FragmentShOut out = fragment_sh(vao);

                    draw_buf[idx] = out.color;
                    z_buf[idx] = z;
                }
            }
        }
    }

    prof_stop(SLOT_WA_RENDER);
}

float wa_fline(float x, float y, float px, float py, float qx, float qy) {
    return (py - qy) * x + (qx - px) * y + px * qy - qx * py;
}

void wa_draw_line(float x0, float y0, float xf, float yf, RGBA c0, RGBA cf) {
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
