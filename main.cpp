#include <stdio.h>
#include <stdlib.h>

#include <pspkernel.h>

#include "wa.hpp"

PSP_MODULE_INFO("WA", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

i32 exit_request = 0;

SceUID setup_callbacks();
int callback_thread(SceSize args, void *argp);
int exit_callback(int arg1, int arg2, void *common);

int main() {
    RGBA g = {.r = 127, .g = 127, .b = 127, .a = 255};

    float fov = 60.0f;
    float n = 0.001f;
    float f = 100.0f;
    V3f eye = {.x = 0, .y = 0, .z = 10};
    V3f at = {.x = 0, .y = 0, .z = 0};
    V3f up = wa_up();

    V3f vs_[] = {
        {.v = {1.0f, -1.0f, -1.0f}}, {.v = {1.0f, -1.0f, 1.0f}},
        {.v = {-1.0f, -1.0f, 1.0f}}, {.v = {-1.0f, -1.0f, -1.0f}},
        {.v = {1.0f, 1.0f, -1.0}},   {.v = {1.0, 1.0f, 1.0f}},
        {.v = {-1.0f, 1.0f, 1.0f}},  {.v = {-1.0f, 1.0f, -1.0f}},
    };
    V2f uvs_[] = {
        {.v = {0.0f, 0.0f}}, {.v = {0.0f, 0.0f}}, {.v = {0.0f, 0.0f}},
        {.v = {0.0f, 0.0f}}, {.v = {0.0f, 0.0f}}, {.v = {0.0f, 0.0f}},
        {.v = {0.0f, 0.0f}}, {.v = {0.0f, 0.0f}},
    };
    RGBA cs_[] = {
        {.r = 255, .g = 0, .b = 0, .a = 255},
        {.r = 255, .g = 0, .b = 0, .a = 255},
        {.r = 255, .g = 0, .b = 0, .a = 255},
        {.r = 255, .g = 0, .b = 0, .a = 255},
        {.r = 255, .g = 0, .b = 0, .a = 255},
        {.r = 255, .g = 0, .b = 0, .a = 255},
        {.r = 255, .g = 0, .b = 0, .a = 255},
        {.r = 255, .g = 0, .b = 0, .a = 255},
    };
    V3i ts_[] = {
        {.v = {2 - 1, 3 - 1, 4 - 1}}, {.v = {8 - 1, 7 - 1, 6 - 1}},
        {.v = {5 - 1, 6 - 1, 2 - 1}}, {.v = {6 - 1, 7 - 1, 3 - 1}},
        {.v = {3 - 1, 7 - 1, 8 - 1}}, {.v = {1 - 1, 4 - 1, 8 - 1}},
        {.v = {1 - 1, 2 - 1, 4 - 1}}, {.v = {5 - 1, 8 - 1, 6 - 1}},
        {.v = {1 - 1, 5 - 1, 2 - 1}}, {.v = {2 - 1, 6 - 1, 3 - 1}},
        {.v = {4 - 1, 3 - 1, 8 - 1}}, {.v = {5 - 1, 1 - 1, 8 - 1}},
    };

    M4f m = M4f::I();
    M4f v = wa_look_at(eye, at, up);
    M4f p = wa_perspective_fov((fov * M_PI) / 180.0f, n, f);

    BufC<V3f> vs = BUF_FROM_C_ARR(vs_);
    BufC<V2f> uvs = BUF_FROM_C_ARR(uvs_);
    BufC<RGBA> cs = BUF_FROM_C_ARR(cs_);
    BufC<V3i> ts = BUF_FROM_C_ARR(ts_);
    VertexSh_fp vertex_sh = wa_vertex_sh_basic;
    FragmentSh_fp fragment_sh = wa_fragment_sh_basic;

    setup_callbacks();
    i32 ok = wa_init();
    MUST(ok);

    while (!exit_request) {
        wa_clear(g);
        wa_render(m, v, p, vs, uvs, cs, ts, vertex_sh, fragment_sh);

        wa_swap_bufs();
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

// See c.h::MUST
void must_cb(const char *expr, const char *file, i32 line) {
    // NOTE: fprintf prints a newline after each argument

    char buf[256];
    snprintf(buf, 256, "Failed MUST %s:%ld: %s\n", file, line, expr);
    fputs(buf, stderr);
    fflush(stderr);

    sceKernelExitGame();
}

// See c.h::ASSERTZ
void assertz_cb(const char *expr, const char *file, i32 line) {
    // NOTE: fprintf prints a newline after each argument

    char buf[256];
    snprintf(buf, 256, "Failed ASSERTZ %s:%ld: %s\n", file, line, expr);
    fputs(buf, stderr);
}
