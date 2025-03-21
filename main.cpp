#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <pspkernel.h>

#include "vfpu.hpp"
#include "wa.hpp"

enum ExitCode {
    OK,
    INIT_ERR,
    CLEAR_ERR,
};

i32 exit_request = 0;

SceUID setup_callbacks();
int callback_thread(SceSize args, void *argp);
int exit_callback(int arg1, int arg2, void *common);

int main() {
    RGBA g = {.ptr = {127, 127, 127, 255}};

    float fov = 60.0f;
    float n = 1.0f;
    float f = 10.0f;
    V3f eye = {0, 0, 0};
    V3f at = {0, 0, 0};
    V3f up = wa_up();

    V3f vs_[] = {
        {0, 1, 0},
        {cosf(M_PI / 6.0f), -1, sinf(M_PI / 6.0f)},
        {cosf(5 * M_PI / 6.0f), -1, sinf(5 * M_PI / 6.0f)},
        {cosf(9 * M_PI / 6.0f), -1, sinf(9 * M_PI / 6.0f)},
    };
    RGBA cs_[] = {
        {0, 0, 0, 255},
        {255, 0, 0, 255},
        {0, 255, 0, 255},
        {0, 0, 255, 255},
    };
    V3i ts_[] = {
        {0, 1, 2},
        {0, 2, 3},
        {0, 3, 1},
        {1, 2, 3},
    };

    VFPU_ALIGNED M4f m = M4f::I();
    VFPU_ALIGNED M4f p = wa_perspective_fov((fov * M_PI) / 180.0f, n, f);

    Buf<V3f> vs = BUF_FROM_C_ARR(vs_);
    Buf<RGBA> cs = BUF_FROM_C_ARR(cs_);
    Buf<V3i> ts = BUF_FROM_C_ARR(ts_);
    VertexSh_fp vertex_sh = wa_vertex_sh_basic;
    FragmentSh_fp fragment_sh = wa_fragment_sh_basic;

    setup_callbacks();
    i32 ok = wa_init();
    ASSERTC(ok, ExitCode::INIT_ERR);

    float elapsed = 0;
    clock_t t = clock();
    while (!exit_request) {
        clock_t tf = clock();
        elapsed += (float)(tf - t) / (float)CLOCKS_PER_SEC;
        t = tf;

        i32 ok = wa_clear(g);
        ASSERTC(ok, ExitCode::CLEAR_ERR);

        eye.x() = 5 * cosf(elapsed);
        eye.y() = 5 * sinf(elapsed + M_PI);
        eye.z() = -(5 * sinf(elapsed));
        VFPU_ALIGNED M4f v = wa_look_at(eye, at, up);

        wa_render(m, v, p, vs, cs, ts, vertex_sh, fragment_sh);

        wa_swap_bufs();
    }

    sceKernelExitGame();
    return ExitCode::OK;
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
