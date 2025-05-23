#include <stdio.h>
#include <stdlib.h>

#include "wv_obj.hpp"

V3i wv_obj_read_f(FILE *fd);

template <i32 N, typename T>
void wv_obj_read_v(FILE *fd, Buf<Arr<N, T>> &buf, i32 idx);

i32 wv_obj_read(const char *file, WvObj *out) {
    ASSERTZ(file != NULL);
    ASSERTZ(out != NULL);

    FILE *fd = fopen(file, "r");
    ASSERTZ(fd != NULL);

    i32 o_cnt = 0;
    i32 v_cnt = 0;
    i32 vn_cnt = 0;
    i32 vt_cnt = 0;
    i32 f_cnt = 0;

    for (int c = fgetc(fd); c != EOF; c = fgetc(fd)) {
        while (c != '\n') {
            c = fgetc(fd);
        }
        c = fgetc(fd);

        if (c == 'o') {
            o_cnt += 1;
        } else if (c == 'v') {
            c = fgetc(fd);
            if (c == ' ') {
                ++v_cnt;
            } else if (c == 'n') {
                ++vn_cnt;
            } else if (c == 't') {
                ++vt_cnt;
            }
        } else if (c == 'f') {
            ++f_cnt;
        }
    }

    ASSERTZ(o_cnt == 1);

    int fseek_rc = fseek(fd, 0, SEEK_SET);
    ASSERTZ(fseek_rc == 0);

    i32 tri_idx = 0;
    i32 tris_len = f_cnt;
    V3i *tris_ptr = (V3i *)malloc(tris_len * sizeof(V3i));
    i32 verts_idx = 0;
    i32 verts_len = 3 * tris_len;
    WvVertex *verts_ptr = (WvVertex *)malloc(verts_len * sizeof(WvVertex));

    out->tris.ptr = tris_ptr;
    out->tris.len = tris_len;
    out->verts.ptr = verts_ptr;
    out->verts.len = verts_len;

    V3f *v_ptr = (V3f *)malloc(v_cnt * sizeof(V3f));
    V3f *vn_ptr = (V3f *)malloc(vn_cnt * sizeof(V3f));
    V2f *vt_ptr = (V2f *)malloc(vt_cnt * sizeof(V2f));

    i32 v_idx = 0;
    Buf<V3f> v_buf = {v_ptr, v_cnt};
    i32 vn_idx = 0;
    Buf<V3f> vn_buf = {vn_ptr, vn_cnt};
    i32 vt_idx = 0;
    Buf<V2f> vt_buf = {vt_ptr, vt_cnt};

    if (tris_ptr == NULL || verts_ptr == NULL || v_ptr == NULL ||
        vn_ptr == NULL || vt_ptr == NULL) {
        goto __wv_obj_read_cleanup;
    }

    for (int c = fgetc(fd); c != EOF; c = fgetc(fd)) {
        while (c != '\n') {
            c = fgetc(fd);
        }
        c = fgetc(fd);

        if (c == 'v') {
            c = fgetc(fd);
            if (c == ' ') {
                wv_obj_read_v(fd, v_buf, v_idx);
                ++v_idx;
            } else if (c == 'n') {
                fgetc(fd); // Consume " "
                wv_obj_read_v(fd, vn_buf, vn_idx);
                ++vn_idx;
            } else if (c == 't') {
                fgetc(fd); // Consume " "
                wv_obj_read_v(fd, vt_buf, vt_idx);
                ++vt_idx;
            }
        } else if (c == 'f') {
            for (i32 i = 0; i < 3; ++i) {
                fgetc(fd); // Consume " "
                V3i f = wv_obj_read_f(fd);
                i32 f_v_idx = f.x();
                i32 f_vn_idx = f.y();
                i32 f_vt_idx = f.z();
                ASSERTZ(f_v_idx < v_idx);
                ASSERTZ(f_vn_idx < vn_idx);
                ASSERTZ(f_vt_idx < vt_idx);

                V3f &v = v_buf[f_v_idx];
                V3f &vn = vn_buf[f_vn_idx];
                V2f &vt = vt_buf[f_vt_idx];

                out->verts[verts_idx + i] = {v, vn, vt};
            }

            out->tris[tri_idx] = {verts_idx, verts_idx + 1, verts_idx + 2};

            tri_idx += 1;
            verts_idx += 3;
        }
    }
    goto __wv_obj_read_cleanup_succ;

__wv_obj_read_cleanup:
    free(tris_ptr);
    free(verts_ptr);
__wv_obj_read_cleanup_succ:
    free(v_ptr);
    free(vn_ptr);
    free(vt_ptr);
    int fclose_rc = fclose(fd);

    ASSERTZ(tris_ptr != NULL);
    ASSERTZ(verts_ptr != NULL);
    ASSERTZ(v_ptr != NULL);
    ASSERTZ(vn_ptr != NULL);
    ASSERTZ(vt_ptr != NULL);
    ASSERTZ(fclose_rc == 0);

    return 1;
}

V3i wv_obj_read_f(FILE *fd) {
    MUST(fd != NULL);

#ifndef NDEBUG
    int c = fgetc(fd);
    MUST(c >= '0' && c <= '9');
    MUST(ungetc(c, fd) != EOF);
#endif

    i32 v_idx;
    i32 vn_idx;
    i32 vt_idx;
    int fscanf_rc = fscanf(fd, "%ld/%ld/%ld", &v_idx, &vt_idx, &vn_idx);
    MUST(fscanf_rc != EOF);

    // NOTE: 1-indexed XD
    // NOTE: v/vn/vt is the order in the file but v/vt/vn is the order in f XD
    V3i f = {v_idx - 1, vn_idx - 1, vt_idx - 1};
    return f;
}

template <i32 N, typename T>
void wv_obj_read_v(FILE *fd, Buf<Arr<N, T>> &buf, i32 idx) {
    MUST(fd != NULL);
    MUST(N == 2 || N == 3);
    MUST(c_arr_idx_check(buf.ptr, buf.len, idx));

#ifndef NDEBUG
    int c = fgetc(fd);
    MUST(c == '-' || (c >= '0' && c <= '9'));
    MUST(ungetc(c, fd) != EOF);
#endif

    for (i32 i = 0; i < N; ++i) {
        float x;
        int fscanf_rc = fscanf(fd, "%f", &x);
        MUST(fscanf_rc != EOF);

        buf[idx][i] = x;
    }
}
