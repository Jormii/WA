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

i32 wa_clear(RGBA color) {
#define CLEAR_MAT 0
#define CLEAR_ROW 0

    RGBA *ptr = draw_buf;
    const RGBA *END = ptr + FRAME_BUF_SIZE;

    color.a = ALPHA_CLEAR;
    i32 c = color.rgba;
    VFPU_ALIGNED i32 row[] = {c, c, c, c};
    VFPU_LOAD_V4_ROW(CLEAR_MAT, CLEAR_ROW, row, 0);

    for (; ptr != END; ptr += 16) {
        VFPU_STORE_V4_ROW(CLEAR_MAT, CLEAR_ROW, ptr, 0);
        VFPU_STORE_V4_ROW(CLEAR_MAT, CLEAR_ROW, ptr, 16);
        VFPU_STORE_V4_ROW(CLEAR_MAT, CLEAR_ROW, ptr, 32);
        VFPU_STORE_V4_ROW(CLEAR_MAT, CLEAR_ROW, ptr, 48);
    }

    return 1;

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

    M4f r = rotation_m(x, y, z);
    M4f t = translation_m(-eye);
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
    *(orthographic.get(0, 0)) = 2.0f / (r - l);
    *(orthographic.get(1, 1)) = 2.0f / (t - b);
    *(orthographic.get(2, 2)) = -2.0f / (f - n);

    return orthographic;
}

M4f wa_perspective(float l, float r, float b, float t, float n, float f) {
    MUST(l < r);
    MUST(b < t);
    MUST(n > 0 && n < f);

    M4f perspective = M4f::zeros();
    *(perspective.get(0, 0)) = (2.0f * n) / (r - l);
    *(perspective.get(0, 2)) = (r + l) / (r - l);
    *(perspective.get(1, 1)) = (2.0f * n) / (t - b);
    *(perspective.get(1, 2)) = (t + b) / (t - b);
    *(perspective.get(2, 2)) = -(f + n) / (f - n);
    *(perspective.get(2, 3)) = (-2.0f * f * n) / (f - n);
    *(perspective.get(3, 2)) = -1;

    return perspective;
}

M4f wa_perspective_fov(float fov, float n, float f) {
    MUST(fov > 0 && fov < M_PI_2);

    float t = n * tanf(0.5f * fov);
    float r = ((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT) * t;

    return wa_perspective(-r, r, -t, t, n, f);
}

void wa_render(                                      //
    const M4f &m, const M4f &v, const M4f &p,        //
    Buf<V3f> vs, Buf<RGBA> cs, Buf<V3i> ts,          //
    VertexSh_fp vertex_sh, FragmentSh_fp fragment_sh //
) {
    M3f w = wa_viewport();
    M4f mv = v * m;
    M4f mvp = p * mv;

    for (i32 i = 0; i < ts.len; ++i) {
        V3f screen[3];
        RGBA screen_colors[3];
        const V3i &triangle = *ts.get(i);

        for (i32 j = 0; j < 3; ++j) {
            i32 vertex_idx = *triangle.get(j);

            V3f vertex = *vs.get(vertex_idx);
            RGBA color = *cs.get(vertex_idx);

            VertexShIn in = {vertex, color, mvp};
            VertexShOut out = vertex_sh(in);

            V3f canonical = persp_div(out.position);
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
            y0 = min(y0, screen[j].y());
            xf = max(xf, screen[j].x());
            yf = max(yf, screen[j].y());
        }

        x0 = max(x0, VIEWPORT_LEFT);
        xf = min(xf, VIEWPORT_RIGHT);
        y0 = max(y0, VIEWPORT_TOP);
        yf = min(yf, VIEWPORT_BOTTOM);

#if 1
        V3f *_s = screen;
        RGBA _b = {.rgba = 0xFFFFFFFF};
        wa_draw_line(_s[0].x(), _s[0].y(), _s[1].x(), _s[1].y(), _b, _b);
        wa_draw_line(_s[1].x(), _s[1].y(), _s[2].x(), _s[2].y(), _b, _b);
        wa_draw_line(_s[2].x(), _s[2].y(), _s[0].x(), _s[0].y(), _b, _b);
#endif

        for (float y = y0; y <= yf; ++y) {
            for (float x = x0; x <= xf; ++x) {
                V3f &sx = screen[0];
                V3f &sy = screen[1];
                V3f &sz = screen[2];

                float alpha =
                    wa_fline(x, y, sy.x(), sy.y(), sz.x(), sz.y()) /
                    wa_fline(sx.x(), sx.y(), sy.x(), sy.y(), sz.x(), sz.y());
                float beta =
                    wa_fline(x, y, sz.x(), sz.y(), sx.x(), sx.y()) /
                    wa_fline(sy.x(), sy.y(), sz.x(), sz.y(), sx.x(), sx.y());
                float gamma = 1 - (alpha + beta);

                if (alpha > 0 && beta > 0 && gamma > 0) {
                    i32 idx = wa_buf_idx(x, y);

                    float z = Z_BUF_CLEAR;
                    const float *z_out = bary_v(   //
                        &sx.z(), &sy.z(), &sz.z(), //
                        alpha, beta, gamma,        //
                        &z, 1                      //
                    );
                    MUST(z_out != NULL);

                    if (z < Z_BUF_MAX || z > Z_BUF_CLEAR) {
                        continue;
                    }

                    float __alpha = map_range( //
                        z, Z_BUF_MAX, Z_BUF_CLEAR, ALPHA_MAX, ALPHA_CLEAR);
                    MUST(__alpha >= ALPHA_MAX);
                    MUST(__alpha <= ALPHA_CLEAR);

                    u8 a = (u8)__alpha;
                    if (draw_buf[idx].a < a) {
                        continue;
                    }

                    RGBA color = RGBA::bary(                                  //
                        screen_colors[0], screen_colors[1], screen_colors[2], //
                        alpha, beta, gamma                                    //
                    );

                    FragmentShIn in = {color};
                    FragmentShOut out = fragment_sh(in);

                    color = out.color;
                    color.a = a;
                    draw_buf[idx] = color;
                }
            }
        }
    }
}

float wa_fline(float x, float y, float px, float py, float qx, float qy) {
    return (py - qy) * x + (qx - px) * y + px * qy - qx * py;
}

void wa_draw_line(float x0, float y0, float xf, float yf, RGBA c0, RGBA cf) {
    c0.a = ALPHA_MAX;
    cf.a = ALPHA_MAX;

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
        }
    }
}

VertexShOut wa_vertex_sh_basic(const VertexShIn &in) {
    const V3f &v = in.vertex;
    V4f position = {v.x(), v.y(), v.z(), 1.0f};
    V4f out_position = in.mvp * position;

    VertexShOut out = {out_position, in.color};
    return out;
}

FragmentShOut wa_fragment_sh_basic(const FragmentShIn &in) {
    FragmentShOut out = {in.color};
    return out;
}
