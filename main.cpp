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

struct Vertex {
    V3f vertex;
    RGBA color;
};
static_assert(sizeof(Vertex) == sizeof(V4f));

i32 exit_request = 0;

SceUID setup_callbacks();
int callback_thread(SceSize args, void *argp);
int exit_callback(int arg1, int arg2, void *common);

VertexShOut vertex_sh(i32 vertex_idx, const VAO &vao) {
    const M4f &mvp = vao.unif_m4f(0);
    const V3f &vertex = vao.attr_v3f(0, vertex_idx);
    const RGBA &color = vao.attr_rgba(1, vertex_idx);

    V4f homogeneous = v4_point(vertex);
    V4f out_vertex = mvp * homogeneous;

    RGBA out_color = color;

    return {out_vertex, out_color};
}

FragmentShOut fragment_sh(const RGBA &color) {
    RGBA out_color = color;
    return {out_color};
}

int main() {
    prof_rename(SLOT_LOOP, "loop");
    prof_rename(SLOT_WA_CLEAR, "wa_clear");
    prof_rename(SLOT_WA_RENDER, "wa_render");

    RGBA g = {.ptr = {127, 127, 127, 255}};

    float fov = 60.0f;
    float n = 1.0f;
    float f = 10.0f;
    V3f eye = {0, 0, 0};
    V3f at = {0, 0, 0};
    V3f up = wa_up();

    V3i triangles_[] = {
        {0, 1, 2},
        {0, 2, 3},
        {0, 3, 1},
        {1, 2, 3},
    };
    Vertex vertices_[] = {
        {{0, 1, 0}, {255, 255, 255, 255}},                                //
        {{cosf(M_PI / 6), -1, sinf(M_PI / 6)}, {255, 0, 0, 255}},         //
        {{cosf(5 * M_PI / 6), -1, sinf(5 * M_PI / 6)}, {0, 255, 0, 255}}, //
        {{cosf(9 * M_PI / 6), -1, sinf(9 * M_PI / 6)}, {0, 0, 255, 255}}, //
    };

    VFPU_ALIGNED M4f m = M4f::I();
    VFPU_ALIGNED M4f p = wa_perspective_fov((fov * M_PI) / 180.0f, n, f);

    Buf<V3i> triangles = BUF_FROM_C_ARR(triangles_);
    Buf<Vertex> vertices = BUF_FROM_C_ARR(vertices_);

    i32 n_ptrs = 2;  // {vertices} and {mvp} (below)
    i32 n_unifs = 1; // {mvp}
    i32 n_attrs = 2; // Position and color
    VAO vao = VAO::alloc(n_ptrs, n_unifs, n_attrs);

    // vao.ptr(0, mvp.ptr); // IS SET BELOW
    vao.ptr(1, vertices.ptr);

    vao.unif(0, 0, VAOType::M4f);
    vao.attr(1, 0, 0, sizeof(Vertex), VAOType::V3f);
    vao.attr(1, 1, sizeof(Vertex::vertex), sizeof(Vertex), VAOType::RGBA);

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

        eye.x() = 5 * cosf(elapsed);
        eye.y() = 5 * sinf(elapsed + M_PI);
        eye.z() = -(5 * sinf(elapsed));
        VFPU_ALIGNED M4f v = wa_look_at(eye, at, up);
        VFPU_ALIGNED M4f mv = v * m;
        VFPU_ALIGNED M4f mvp = p * mv;

        vao.ptr(0, &mvp);
        wa_render(vao, triangles, vertex_sh, fragment_sh);

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
