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

// clang-format off
void wa_render(
    const M4f &m, const M4f &v, const M4f &p,
    BufC<V3f> vs, BufC<V2f> uvs, BufC<RGBA> cs, BufC<V3i> ts,
    VertexSh_fp vertex_sh, FragmentSh_fp fragment_sh
) {
    // clang-format on

    M3f w = wa_viewport();
    M4f mv = M4f::mmult(v, m);
    M4f mvp = M4f::mmult(p, mv);

    for (i32 i = 0; i < ts.len; ++i) {
        V3f screen[3];
        RGBA screen_colors[3];
        const V3i &triangle = ts[i];

        for (i32 j = 0; j < 3; ++j) {
            i32 vertex_idx = triangle.v[j];

            V3f vertex = vs[vertex_idx];
            V2f uv = uvs[vertex_idx];
            RGBA color = cs[vertex_idx];

            VertexShIn in = {
                .vertex = vertex, .uv = uv, .color = color, .mvp = mvp};
            VertexShOut out = vertex_sh(in);

            V3f canonical = out.position.persp_div();
            V3f canonical_xy = {.v = {canonical.x, canonical.y, 1}};
            screen[j] = w * canonical_xy;
        }

        float x0, y0;
        float xf, yf;
        x0 = xf = screen[0].x;
        y0 = yf = screen[0].y;
        for (i32 j = 1; j < 3; ++j) {
            x0 = min(x0, screen[j].x);
            y0 = min(y0, screen[j].y);
            xf = max(xf, screen[j].x);
            yf = max(yf, screen[j].y);
        }

        for (float y = y0; y <= yf; ++y) {
            for (float x = x0; x <= xf; ++x) {
                if (!wa_buf_in(x, y)) {
                    continue;
                }

                float alpha = wa_fline(x, y, screen[1].x, screen[1].y,
                                       screen[2].x, screen[2].y) /
                              wa_fline(screen[0].x, screen[0].y, screen[1].x,
                                       screen[1].y, screen[2].x, screen[2].y);
                float beta = wa_fline(x, y, screen[2].x, screen[2].y,
                                      screen[0].x, screen[0].y) /
                             wa_fline(screen[1].x, screen[1].y, screen[2].x,
                                      screen[2].y, screen[0].x, screen[0].y);
                float gamma = 1 - (alpha + beta);
                if (alpha > 0 && beta > 0 && gamma > 0) {
                    V2f uv = V2f::bary(uvs[triangle.v[0]], uvs[triangle.v[1]],
                                       uvs[triangle.v[2]], alpha, beta, gamma);
                    RGBA color =
                        RGBA::mix_bary(screen_colors[0], screen_colors[1],
                                       screen_colors[2], alpha, beta, gamma);

                    FragmentShIn in = {.uv = uv, .color = color};
                    FragmentShOut out = fragment_sh(in);

                    i32 idx = wa_buf_idx(x, y);
                    draw_buf[idx] = out.color;
                }
            }
        }
    }
}

float wa_fline(float x, float y, float px, float py, float qx, float qy) {
    return (py - qy) * x + (qx - px) * y + px * qy - qx * py;
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

VertexShOut wa_vertex_sh_basic(const VertexShIn &in) {
    const V3f &v = in.vertex;
    V4f position = {.x = v.x, .y = v.y, .z = v.z, .w = 1.0f};

    VertexShOut out = {.position = position, .color = in.color};
    return out;
}

FragmentShOut wa_fragment_sh_basic(const FragmentShIn &in) {
    FragmentShOut out = {.color = in.color};
    return out;
}
