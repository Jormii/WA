#pragma once

#include "cpp.hpp"

struct WvVertex {
    V3f pos;
    V3f norm;
    V2f uv;
};

struct WvObj {
    Buf<V3i> tris;
    Buf<WvVertex> verts;
};

[[nodiscard]] i32 wv_obj_read(const char *file, WvObj *out);
