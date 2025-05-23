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
#include "wv_obj.hpp"

enum __VAOBuf {
    BUF_V,
    BUF_MVP,
    __BUF_CNT,
};

enum __VAOIn {
    IN_UV,
    IN_POS,
    IN_NORM,
    __IN_CNT,
};

enum __VAOUnif {
    UNIF_MVP,
    __UNIF_CNT,
};

enum __VAOOut {
    OUT_UV,
    OUT_NORM,
    __OUT_CNT,
};

i32 exit_request = 0;

SceUID setup_callbacks();
int callback_thread(SceSize args, void *argp);
int exit_callback(int arg1, int arg2, void *common);

VertexShOut vertex_sh(i32 v_idx, i32 tri_v_idx, const VAO &vao) {
    const M4f &mvp = *vao.unif_m4f(UNIF_MVP);
    const V2f &uv = vao.in_v2f(IN_UV, v_idx);
    const V3f &pos = vao.in_v3f(IN_POS, v_idx);
    const V3f &norm = vao.in_v3f(IN_NORM, v_idx);

    V4f mvp_v = mvp * v4_point(pos);

    vao.out_v2f(OUT_UV, tri_v_idx) = uv;
    vao.out_v3f(OUT_NORM, tri_v_idx) = norm;

    return {mvp_v};
}

FragmentShOut fragment_sh(const VAO &vao) {
    const V2f &uv = vao.out_bary_v2f(OUT_UV);
    const V3f &norm = vao.out_bary_v3f(OUT_NORM);

    V3f n = norm.norm();
    // V4f color_v4 = {uv.x(), uv.y(), 0, 1};
    V4f color_v4 = {fabsf(norm.x()), fabsf(norm.y()), fabsf(norm.z()), 1};

    i32 discard = 0;
    return {color_v4, discard};
}

int main() {
    prof_rename(SLOT_LOOP, "loop");
    prof_rename(SLOT_WA_CLEAR, "wa_clear");
    prof_rename(SLOT_WA_RENDER, "wa_render");
    prof_rename(SLOT_WA_RENDER_SHADOW, "wa_render_shadow");

    const char *file = "umd0:/Untitled.obj";

    WvObj wv_obj;
    FrontFace front = FrontFace::BACKFACE;
    i32 wv_obj_ok = wv_obj_read(file, &wv_obj);
    MUST(wv_obj_ok);

    float dist_to_origin = 5.0f;
    RGBA g = {.ptr = {127, 127, 127, 255}};

    float fov = 60.0f;
    float n = 1.0f;
    float f = 10.0f;
    V3f eye = {0, 0, 0};
    V3f at = {0, 0, 0};
    V3f up = wa_up();

    VFPU_ALIGNED M4f m = M4f::I();
    VFPU_ALIGNED M4f p = wa_perspective_fov((fov * M_PI) / 180.0f, n, f);

    i32 n_bufs = __BUF_CNT;
    i32 n_ins = __IN_CNT;
    i32 n_unifs = __UNIF_CNT;
    VAOType outs_ts_[] = {VAOType::V2f, VAOType::V3f};
    static_assert(C_ARR_LEN(outs_ts_) == __OUT_CNT);

    Buf<VAOType> outs_ts = BUF_FROM_C_ARR(outs_ts_);
    VAO vao = VAO::alloc(n_bufs, n_ins, n_unifs, outs_ts);

    vao.unif(BUF_MVP, UNIF_MVP, VAOType::M4f);

    vao.in(                                                         //
        BUF_V, IN_UV,                                               //
        MEMBER_OFFSET(WvVertex, uv), sizeof(WvVertex), VAOType::V2f //
    );
    vao.in(                                                          //
        BUF_V, IN_POS,                                               //
        MEMBER_OFFSET(WvVertex, pos), sizeof(WvVertex), VAOType::V3f //
    );
    vao.in(                                                           //
        BUF_V, IN_NORM,                                               //
        MEMBER_OFFSET(WvVertex, norm), sizeof(WvVertex), VAOType::V3f //
    );

    setup_callbacks();
    i32 ok = wa_init();
    if (!ok) {
        return 1;
    }

    float elapsed = 0;
    clock_t t = clock();
    while (!exit_request) {
#ifdef PPSSPP
        if (elapsed > 30.0f) {
            break;
            // TODO: For gprof's sake. Something more sophisticated eventually
        }
#endif

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

        vao.buf(BUF_V, wv_obj.verts.ptr, wv_obj.verts.len);
        wa_render(vao, wv_obj.tris, front, vertex_sh, fragment_sh);

        wa_swap_bufs();
        sceDisplayWaitVblankStart();

        prof_stop(SLOT_LOOP);
        prof_dump();
    }

#ifndef PPSSPP
    sceKernelExitGame();
// NOTE: PPSSPP abruptly ends the emulation thus interfering with gprof
#endif
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
