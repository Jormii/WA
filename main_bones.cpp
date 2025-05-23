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

enum __VAOBuf {
    BUF_V,
    BUF_MVP,
    BUF_SKELETON,
    __BUF_CNT,
};

enum __VAOIn {
    IN_V,
    IN_N,
    __IN_CNT,
};

enum __VAOUnif {
    UNIF_MVP,
    UNIF_SKELETON,
    __UNIF_CNT,
};

enum __VAOOut {
    OUT_N,
    __OUT_CNT,
};

struct Vertex {
    V3f vertex;
    V3f normal;
};

i32 exit_request = 0;

SceUID setup_callbacks();
int callback_thread(SceSize args, void *argp);
int exit_callback(int arg1, int arg2, void *common);

VertexShOut vertex_sh(i32 v_idx, i32 tri_v_idx, const VAO &vao) {
    const M4f &mvp = *vao.unif_m4f(UNIF_MVP);
    const Skeleton &skeleton = *vao.unif_skeleton(UNIF_SKELETON);
    const V3f &vertex = vao.in_v3f(IN_V, v_idx);
    const V3f &normal = vao.in_v3f(IN_N, v_idx);

    M4f m = M4f::zeros();
    for (i32 bone_idx = 0; bone_idx < skeleton.bones.len; ++bone_idx) {
        const Bone &bone = skeleton.bones[bone_idx];
        const float weight = skeleton.weights.get(v_idx, bone_idx);

        m += weight * bone.transform_global;
    }

    V4f out_n = m * v4_vector(normal);
    V4f out_v = mvp * (m * v4_point(vertex));

    vao.out_v3f(OUT_N, tri_v_idx) = out_n.xyz();

    return {out_v};
}

FragmentShOut fragment_sh(const VAO &vao) {
    const V3f &normal = vao.out_bary_v3f(OUT_N);

    V3f n = normal.norm();

    V4f color_v4 = {fabsf(n.x()), fabsf(n.y()), fabsf(n.z()), 1};
    i32 discard = 0;
    return {color_v4, discard};
}

int main() {
    prof_rename(SLOT_LOOP, "loop");
    prof_rename(SLOT_WA_CLEAR, "wa_clear");
    prof_rename(SLOT_WA_RENDER, "wa_render");
    prof_rename(SLOT_WA_RENDER_SHADOW, "wa_render_shadow");

    float dist_to_origin = 3.0f;
    RGBA g = {.ptr = {127, 127, 127, 255}};

    float fov = 60.0f;
    float n = 1.0f;
    float f = 10.0f;
    V3f eye = {dist_to_origin, -0.5, dist_to_origin};
    V3f at = {0, -0.5, 0};
    V3f up = wa_up();

    V3f positions[] = {
        {-0.5, 0, 0}, //
        {+0.5, 0, 0}, //
        {-0.5, 1, 0}, //
        {+0.5, 1, 0}, //
        {-0.5, 2, 0}, //
        {+0.5, 2, 0}, //
        {-0.5, 3, 0}, //
        {+0.5, 3, 0}, //
        {-0.5, 4, 0}, //
        {+0.5, 4, 0}, //
    };
    V3f normals[] = {
        {0, 0, 1}, //
    };

    FrontFace front = FrontFace::BACKFACE;
    V3i triangles_[] = {
        {0, 1, 3}, //
        {0, 3, 2}, //
        {2, 3, 5}, //
        {2, 5, 4}, //
        {4, 5, 7}, //
        {4, 7, 6}, //
        {6, 7, 9}, //
        {6, 9, 8}, //
    };
    Vertex vertices_[] = {
        {positions[0], normals[0]}, //
        {positions[1], normals[0]}, //
        {positions[2], normals[0]}, //
        {positions[3], normals[0]}, //
        {positions[4], normals[0]}, //
        {positions[5], normals[0]}, //
        {positions[6], normals[0]}, //
        {positions[7], normals[0]}, //
        {positions[8], normals[0]}, //
        {positions[9], normals[0]}, //
    };

    Bone bones_[4] = {
        Bone::init_I(-1, {0, 0, 0}),
        Bone::init_I(+0, {0, 1, 0}),
        Bone::init_I(+1, {0, 2, 0}),
        Bone::init_I(+2, {0, 3, 0}),
    };
    float weights_[] = {
        1, 0, 0, 0, //
        1, 0, 0, 0, //
        1, 0, 0, 0, //
        1, 0, 0, 0, //
        0, 1, 0, 0, //
        0, 1, 0, 0, //
        0, 0, 1, 0, //
        0, 0, 1, 0, //
        0, 0, 0, 1, //
        0, 0, 0, 1, //
    };

    VFPU_ALIGNED M4f m = M4f::I();
    VFPU_ALIGNED M4f p = wa_perspective_fov((fov * M_PI) / 180.0f, n, f);

    Buf<V3i> triangles = BUF_FROM_C_ARR(triangles_);
    Buf<Vertex> vertices = BUF_FROM_C_ARR(vertices_);
    Buf<Bone> bones = BUF_FROM_C_ARR(bones_);
    Buf2D<float> weights = {weights_, vertices.len, bones.len};

    i32 n_bufs = __BUF_CNT;
    i32 n_ins = __IN_CNT;
    i32 n_unifs = __UNIF_CNT;
    VAOType outs_ts_[] = {VAOType::V3f};
    static_assert(C_ARR_LEN(outs_ts_) == __OUT_CNT);

    Buf<VAOType> outs_ts = BUF_FROM_C_ARR(outs_ts_);
    VAO vao = VAO::alloc(n_bufs, n_ins, n_unifs, outs_ts);

    vao.unif(BUF_MVP, UNIF_MVP, VAOType::M4f);
    vao.unif(BUF_SKELETON, UNIF_SKELETON, VAOType::Skeleton);

    vao.in(                                                         //
        BUF_V, IN_V,                                                //
        MEMBER_OFFSET(Vertex, vertex), sizeof(Vertex), VAOType::V3f //
    );
    vao.in(                                                         //
        BUF_V, IN_N,                                                //
        MEMBER_OFFSET(Vertex, normal), sizeof(Vertex), VAOType::V3f //
    );

    setup_callbacks();
    i32 ok = wa_init();
    if (!ok) {
        return 1;
    }

    VFPU_ALIGNED M4f v = wa_look_at(eye, at, up);
    VFPU_ALIGNED M4f mv = v * m;
    VFPU_ALIGNED M4f mvp = p * mv;
    Skeleton skeleton = Skeleton::init(bones, weights);

    vao.buf(BUF_MVP, &mvp, 1);
    vao.buf(BUF_SKELETON, &skeleton, 1);

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

        float angle = fabsf(sinf(elapsed)) * M_PI_2;
        bones[0].transform = wa_rot_x(angle);
        bones[1].transform = wa_rot_x_and_translate(angle, {0, 1, 0});
        bones[2].transform = wa_rot_x_and_translate(angle, {0, 1, 0});
        bones[3].transform = wa_rot_x_and_translate(angle, {0, 1, 0});
        skeleton.update();

        wa_clear(g);

        vao.buf(BUF_V, vertices.ptr, vertices.len);
        wa_render(vao, triangles, front, vertex_sh, fragment_sh);

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
