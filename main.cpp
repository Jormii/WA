#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <pspdisplay.h>
#include <pspkernel.h>
#pragma GCC diagnostic pop

#include "vfpu.hpp"
#include "wa.hpp"

#include "tex.png.hpp"

enum __VAOBuf {
    BUF_V,
    BUF_MVP,
    BUF_TEXTURE,
    BUF_TEXTURE_COLORS,
    BUF_POINT_LIGHTS,
    __BUF_CNT,
};

enum __VAOIn {
    IN_UV,
    IN_V,
    IN_N,
    IN_COLOR,
    __IN_CNT,
};

enum __VAOUnif {
    UNIF_MVP,
    UNIF_TEXTURE,
    UNIF_TEXTURE_COLORS,
    UNIF_POINT_LIGHTS,
    __UNIF_CNT,
};

enum __VAOOut {
    OUT_UV,
    OUT_V,
    OUT_N,
    OUT_COLOR,
    __OUT_CNT,
};

struct Vertex {
    V2f uv;
    V3f vertex;
    V3f normal;
    RGBA color;
};

i32 exit_request = 0;

SceUID setup_callbacks();
int callback_thread(SceSize args, void *argp);
int exit_callback(int arg1, int arg2, void *common);

VertexShOut vertex_sh(i32 v_idx, i32 tri_v_idx, const VAO &vao) {
    const M4f &mvp = *vao.unif_m4f(UNIF_MVP);
    const V2f &uv = vao.in_v2f(IN_UV, v_idx);
    const V3f &vertex = vao.in_v3f(IN_V, v_idx);
    const V3f &normal = vao.in_v3f(IN_N, v_idx);
    const RGBA &color = vao.in_rgba(IN_COLOR, v_idx);

    V4f mvp_v = mvp * v4_point(vertex);

    vao.out_v2f(OUT_UV, tri_v_idx) = uv;
    vao.out_v3f(OUT_V, tri_v_idx) = vertex;
    vao.out_v3f(OUT_N, tri_v_idx) = normal;
    vao.out_v4f(OUT_COLOR, tri_v_idx) = color.v4f();

    return {mvp_v};
}

FragmentShOut fragment_sh(const VAO &vao) {
    const Texture &texture = *vao.unif_texture(UNIF_TEXTURE);
    const Buf<RGBA> &texture_colors = vao.unif_rgba(UNIF_TEXTURE_COLORS);
    const Buf<PointLight> &lights = vao.unif_point_light(UNIF_POINT_LIGHTS);
    const V2f &uv = vao.out_bary_v2f(OUT_UV);
    const V3f &vertex = vao.out_bary_v3f(OUT_V);
    const V3f &normal = vao.out_bary_v3f(OUT_N);
    // const V4f &color = vao.out_bary_v4f(OUT_COLOR);

    V4f r = texture_colors[0].v4f();
    V4f g = texture_colors[1].v4f();
    V4f b = texture_colors[2].v4f();
    V4f mask = texture_sample(uv, texture).v4f();

    M4f m = {
        r.x(), r.y(), r.z(), r.w(), //
        g.x(), g.y(), g.z(), g.w(), //
        b.x(), b.y(), b.z(), b.w(), //
        1,     1,     1,     1,     //
    };
    V4f color_v4f = m * mask;

    V3f n = normal.norm();
    for (i32 i = 0; i < lights.len; ++i) {
        const PointLight &light = lights[i];

        V3f v = (light.point - vertex).norm();
        float dot = max(0.0f, V3f::dot(n, v));

        color_v4f += dot * light.color;
    }

    RGBA out_color = RGBA::from_v4f(color_v4f);
    return {out_color};
}

int main() {
    prof_rename(SLOT_LOOP, "loop");
    prof_rename(SLOT_WA_CLEAR, "wa_clear");
    prof_rename(SLOT_WA_RENDER, "wa_render");

    float dist_to_origin = 5.0f;
    RGBA g = {.ptr = {127, 127, 127, 255}};

    float fov = 60.0f;
    float n = 1.0f;
    float f = 10.0f;
    V3f eye = {0, 0, 0};
    V3f at = {0, 0, 0};
    V3f up = wa_up();

    V2f uvs[] = {
        {0.997998, 0.501001}, {0.001001, 0.997998}, {0.001001, 0.501001},
        {0.997998, 0.001001}, {0.001001, 0.001001}, {0.001001, 0.497998},
        {0.002002, 0.998999}, {0.998999, 0.502002}, {0.998999, 0.998999},
        {0.002002, 0.498999}, {0.998999, 0.002002}, {0.998999, 0.498999},
    };
    V3f positions[] = {
        {0.000000, -1.000000, -1.000000},
        {0.866025, -1.000000, 0.500000},
        {-0.866025, -1.000000, 0.500000},
        {0.000000, 1.000000, 0.000000},
    };
    V3f normals[] = {
        {0.8402, 0.2425, -0.4851},
        {-0.0000, -1.0000, -0.0000},
        {-0.0000, 0.2425, 0.9701},
        {-0.8402, 0.2425, -0.4851},
    };
    RGBA colors[]{
        {0, 0, 0, 0}, // {255, 0, 0, 255},
        {0, 0, 0, 0}, // {0, 255, 0, 255},
        {0, 0, 0, 0}, // {0, 0, 255, 255},
        {0, 0, 0, 0}, // {255, 255, 255, 255},
    };

    FrontFace front_face = FrontFace::CCW;
    V3i triangles_[] = {
        {0, 1, 2},
        {3, 4, 5},
        {6, 7, 8},
        {9, 10, 11},
    };
    Vertex vertices_[] = {
        {uvs[0], positions[0], normals[0], colors[0]},
        {uvs[1], positions[3], normals[0], colors[3]},
        {uvs[2], positions[1], normals[0], colors[1]},
        //
        {uvs[3], positions[0], normals[1], colors[0]},
        {uvs[4], positions[1], normals[1], colors[1]},
        {uvs[5], positions[2], normals[1], colors[2]},
        //
        {uvs[6], positions[1], normals[2], colors[1]},
        {uvs[7], positions[3], normals[2], colors[3]},
        {uvs[8], positions[2], normals[2], colors[2]},
        //
        {uvs[9], positions[2], normals[3], colors[2]},
        {uvs[10], positions[3], normals[3], colors[3]},
        {uvs[11], positions[0], normals[3], colors[0]},
    };

    float pl_w = 1.2f;
    float pl_y = -1.0f;
    V3f pl_n = {0, 1, 0};
    RGBA pl_rgba = {0, 0, 0, 0};
    FrontFace front_face_pl = FrontFace::CCW;
    V3i triangles_pl_[] = {
        {0, 1, 2},
        {2, 3, 0},
    };
    Vertex vertices_pl_[] = {
        {{0, 1}, {-pl_w, pl_y, -pl_w}, pl_n, pl_rgba},
        {{0, 0}, {-pl_w, pl_y, pl_w}, pl_n, pl_rgba},
        {{1, 0}, {pl_w, pl_y, pl_w}, pl_n, pl_rgba},
        {{1, 1}, {pl_w, pl_y, -pl_w}, pl_n, pl_rgba},
    };

    VFPU_ALIGNED M4f m = M4f::I();
    VFPU_ALIGNED M4f p = wa_perspective_fov((fov * M_PI) / 180.0f, n, f);

    Buf<V3i> triangles = BUF_FROM_C_ARR(triangles_);
    Buf<V3i> triangles_pl = BUF_FROM_C_ARR(triangles_pl_);
    Buf<Vertex> vertices = BUF_FROM_C_ARR(vertices_);
    Buf<Vertex> vertices_pl = BUF_FROM_C_ARR(vertices_pl_);

    i32 n_bufs = __BUF_CNT;
    i32 n_ins = __IN_CNT;
    i32 n_unifs = __UNIF_CNT;
    VAOType outs_ts_[] = {
        VAOType::V2f, //
        VAOType::V3f, //
        VAOType::V3f, //
        VAOType::V4f, //
    };
    static_assert(C_ARR_LEN(outs_ts_) == __OUT_CNT);

    Buf<VAOType> outs_ts = BUF_FROM_C_ARR(outs_ts_);
    VAO vao = VAO::alloc(n_bufs, n_ins, n_unifs, outs_ts);

    Texture texture = tex_texture.buf2d();

    RGBA texture_colors_[] = {
        {123, 45, 67, 0},
        {89, 123, 45, 0},
        {67, 89, 123, 0},
    };
    Buf<RGBA> texture_colors = BUF_FROM_C_ARR(texture_colors_);

    PointLight lights_[] = {
        {{2.0f, 2.0f, 2.0f}, {0.5, 0.5, 0.5, 0}},
    };
    Buf<PointLight> lights = BUF_FROM_C_ARR(lights_);

    vao.buf(BUF_TEXTURE, &texture, 1);
    vao.buf(BUF_TEXTURE_COLORS, texture_colors.ptr, texture_colors.len);
    vao.buf(BUF_POINT_LIGHTS, lights.ptr, lights.len);

    vao.unif(BUF_MVP, UNIF_MVP, VAOType::M4f);
    vao.unif(BUF_TEXTURE, UNIF_TEXTURE, VAOType::Texture);
    vao.unif(BUF_TEXTURE_COLORS, UNIF_TEXTURE_COLORS, VAOType::RGBA);
    vao.unif(BUF_POINT_LIGHTS, UNIF_POINT_LIGHTS, VAOType::PointLight);

    vao.in(                                                     //
        BUF_V, IN_UV,                                           //
        MEMBER_OFFSET(Vertex, uv), sizeof(Vertex), VAOType::V2f //
    );
    vao.in(                                                         //
        BUF_V, IN_V,                                                //
        MEMBER_OFFSET(Vertex, vertex), sizeof(Vertex), VAOType::V3f //
    );
    vao.in(                                                         //
        BUF_V, IN_N,                                                //
        MEMBER_OFFSET(Vertex, normal), sizeof(Vertex), VAOType::V3f //
    );
    vao.in(                                                         //
        BUF_V, IN_COLOR,                                            //
        MEMBER_OFFSET(Vertex, color), sizeof(Vertex), VAOType::RGBA //
    );

    setup_callbacks();
    i32 ok = wa_init();
    if (!ok) {
        return 1;
    }

    float elapsed = 0;
    clock_t t = clock();
    while (!exit_request) {
        prof_kick(SLOT_LOOP);

        clock_t tf = clock();
        elapsed += (float)(tf - t) / (float)CLOCKS_PER_SEC;
        t = tf;

        wa_clear(g);

        eye.x() = dist_to_origin * cosf(elapsed);
        eye.y() = dist_to_origin * sinf(elapsed + M_PI);
        eye.z() = -(dist_to_origin * sinf(elapsed));
        VFPU_ALIGNED M4f v = wa_look_at(eye, at, up);
        VFPU_ALIGNED M4f mv = v * m;
        VFPU_ALIGNED M4f mvp = p * mv;

        vao.buf(BUF_MVP, &mvp, 1);

        vao.buf(BUF_V, vertices.ptr, vertices.len);
        wa_render(vao, triangles, front_face, vertex_sh, fragment_sh);

        vao.buf(BUF_V, vertices_pl.ptr, vertices_pl.len);
        wa_render(vao, triangles_pl, front_face_pl, vertex_sh, fragment_sh);

        wa_swap_bufs();
        sceDisplayWaitVblankStart();

        prof_stop(SLOT_LOOP);
        prof_dump();
    }

    sceKernelExitGame();
    return 0;
}

SceUID setup_callbacks() {
    SceUID id = sceKernelCreateThread("wa", callback_thread, 0x11, 0xFA0, 0, 0);
    if (id >= 0) {
        sceKernelStartThread(id, 0, 0);
    }

    return id;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
int callback_thread(SceSize args, void *argp) {
    int cbid = sceKernelCreateCallback("wa_exit", exit_callback, NULL);
    sceKernelRegisterExitCallback(cbid);

    sceKernelSleepThreadCB();

    return 0;
}
#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
int exit_callback(int arg1, int arg2, void *common) {
    exit_request = 1;
    return 0;
}
#pragma GCC diagnostic pop
